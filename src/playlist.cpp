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
  std::sort(entries.begin(), entries.end(), comparator);
  options.sort_order = sort_order;
}

void Playlist::reload_images_from(const std::vector<std::string>& file_paths) {
  auto t0 = std::chrono::high_resolution_clock::now();

  entries.clear();
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
          entries.emplace_back(new PlaylistEntry {
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
        entries.emplace_back(new PlaylistEntry {
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

  log_debug("Loaded %lu images from %lu files in %lld ms", entries.size(), file_paths.size(), duration_ms);

  set_sort_order(options.sort_order);

  count = entries.size();
}

std::shared_ptr<PlaylistEntry> Playlist::get_current() const {
  if (idx < 0 || idx >= count) {
    return nullptr;
  }

  return entries[idx];
}

std::shared_ptr<PlaylistEntry> Playlist::advance(int by) {
  idx = (idx + by) % count;
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
