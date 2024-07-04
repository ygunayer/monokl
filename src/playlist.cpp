#include "playlist.h"
#include <chrono>
#include <memory>

using namespace monokl;

PlaylistEntryComparator::PlaylistEntryComparator(const PlaylistSortOrder& sort_order)
  : sort_order(sort_order) {
}

bool PlaylistEntryComparator::operator()(const std::shared_ptr<PlaylistEntry>& a, const std::shared_ptr<PlaylistEntry>& b) const {
  switch (sort_order) {
    case PlaylistSortOrderName:
      return a->name < b->name;
    case PlaylistSortOrderNameDesc:
      return a->name > b->name;
    case PlaylistSortOrderDate:
      return a->last_modified_at < b->last_modified_at;
    case PlaylistSortOrderDateDesc:
      return a->last_modified_at > b->last_modified_at;
    default:
      return false;
  }
}

Playlist::Playlist() {
}

void Playlist::set_sort_order(const PlaylistSortOrder& sort_order) {
  PlaylistEntryComparator comparator(sort_order);
  std::sort(all_entries.begin(), all_entries.end(), comparator);
  std::sort(shown_entries.begin(), shown_entries.end(), comparator);
  options.sort_order = sort_order;
}

void Playlist::reload_images_from(const std::vector<std::string>& file_paths) {
  auto t0 = std::chrono::high_resolution_clock::now();

  shown_entries.clear();
  all_entries.clear();
  idx = 0;

  for (const auto& file_path : file_paths) {
    std::filesystem::directory_entry entry(file_path);

    if (entry.is_directory()) {
      for (const auto& child : std::filesystem::directory_iterator(file_path)) {
        if (child.is_directory() || child.is_symlink()) {
          continue;
        }

        auto codec = sail::codec_info::from_path(child.path());
        if (codec.is_valid()) {
          all_entries.emplace_back(new PlaylistEntry {
            .name = child.path().filename().string(),
            .path = child.path().string(),
            .last_modified_at = 0
          });
        }
      }
    } else {
      auto codec = sail::codec_info::from_path(file_path);
      if (codec.is_valid()) {
        std::filesystem::path path(file_path);
        all_entries.emplace_back(new PlaylistEntry {
          .name = path.filename().string(),
          .path = path.string(),
          .last_modified_at = 0
        });
      }
    }
  }

  auto t1 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
  auto duration_ms = static_cast<long long int>(duration.count());

  log_debug("Loaded %lu images from %lu files in %lld ms", all_entries.size(), file_paths.size(), duration_ms);

  set_sort_order(options.sort_order);

  refresh_shown_entries();
}

std::shared_ptr<PlaylistEntry> Playlist::get_current() const {
  if (idx < 0 || idx >= count) {
    return nullptr;
  }
  return shown_entries[idx];
}

unsigned int Playlist::size() const {
  return count;
}

int Playlist::current_index() const {
  return idx;
}

std::shared_ptr<PlaylistEntry> Playlist::advance(int by) {
  idx += by;
  if (idx < 0) {
    idx = count + by;
  } else {
    idx = idx % count;
  }
  return get_current();
}

std::shared_ptr<PlaylistEntry> Playlist::go_to_first() {
  idx = 0;
  return get_current();
}

std::shared_ptr<PlaylistEntry> Playlist::go_to_last() {
  idx = count - 1;
  return get_current();
}

void Playlist::toggle_only_favorites() {
  options.only_favorites = !options.only_favorites;
  refresh_shown_entries();
}

void Playlist::toggle_skip_hidden() {
  options.skip_hidden = !options.skip_hidden;
  refresh_shown_entries();
}

void Playlist::refresh_shown_entries() {
  int prev_idx = idx;
  auto current = get_current();

  int desired_idx = 0;
  int i = 0;
  bool found = false;

  shown_entries.clear();
  for (const auto& entry : all_entries) {
    if (options.only_favorites && !entry->is_favorite) {
      continue;
    }

    if (options.skip_hidden && entry->is_hidden) {
      continue;
    }

    shown_entries.push_back(entry);

    if (entry == current) {
      desired_idx = i;
      found = true;
    }

    i += 1;
  }

  if (!found) {
    desired_idx = prev_idx;
  }

  count = shown_entries.size();

  idx = desired_idx < count ? desired_idx : count - 1;
}
