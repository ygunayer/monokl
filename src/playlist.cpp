#include "playlist.h"
#include <chrono>
#include <memory>
#include <unordered_map>

using namespace monokl;

bool PlaylistEntry::is_folder() const {
  return false;
}

bool FolderEntry::is_folder() const {
  return true;
}

void FolderEntry::toggle_favorite(const std::string& name) {
  if (favorites.find(name) != favorites.end()) {
    favorites.erase(name);
  } else {
    favorites.insert(name);
  }

  settings_changed = true;
}

void FolderEntry::toggle_hidden(const std::string& name) {
  if (hidden.find(name) != hidden.end()) {
    hidden.erase(name);
  } else {
    hidden.insert(name);
  }

  settings_changed = true;
}

void FolderEntry::reload_settings() {
  favorites.clear();
  hidden.clear();

  auto settings_path = path / ".monokl.toml";
  std::filesystem::directory_entry entry(settings_path);
  if (!entry.exists() || !entry.is_regular_file()) {
    return;
  }

  auto data = toml::parse(settings_path);

  auto favorites = toml::find<std::vector<std::string>>(data, "favorites");
  auto hidden = toml::find<std::vector<std::string>>(data, "hidden");

  for (const auto& favorite : favorites) {
    this->favorites.insert(favorite);
  }

  for (const auto& hide : hidden) {
    this->hidden.insert(hide);
  }

  log_debug("Loaded %lu favorites and %lu hidden images for %s", this->favorites.size(), this->hidden.size(), path.string().c_str());
}

void FolderEntry::save_settings() {
  if (!settings_changed) {
    return;
  }

  auto settings_path = path / ".monokl.toml";

  toml::value data;
  data["favorites"] = std::vector<std::string>(favorites.begin(), favorites.end());
  data["hidden"] = std::vector<std::string>(hidden.begin(), hidden.end());

  auto result = toml::format(data);
  std::ofstream file(settings_path);
  file << result;
  file.close();

  log_debug("Saved %lu favorites and %lu hidden images for %s", favorites.size(), hidden.size(), path.string().c_str());
}

bool ImageEntry::is_folder() const {
  return false;
}

bool ImageEntry::is_favorite() const {
  if (parent == nullptr) {
    return false;
  }

  return parent->favorites.find(path.filename()) != parent->favorites.end();
}

bool ImageEntry::is_hidden() const {
  if (parent == nullptr) {
    return false;
  }

  return parent->hidden.find(path.filename()) != parent->hidden.end();
}

PlaylistEntryComparator::PlaylistEntryComparator(const PlaylistSortOrder& sort_order)
  : sort_order(sort_order) {
}

bool PlaylistEntryComparator::operator()(const std::shared_ptr<PlaylistEntry>& a, const std::shared_ptr<PlaylistEntry>& b) const {
  switch (sort_order) {
    case PlaylistSortOrderName:
      return a->path < b->path;
    case PlaylistSortOrderNameDesc:
      return a->path > b->path;
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

  std::unordered_map<std::string, std::shared_ptr<FolderEntry>> tmp_folder_entries;

  for (const auto& file_path : file_paths) {
    std::filesystem::directory_entry entry(file_path);

    if (entry.is_directory()) {
      auto folder = std::make_shared<FolderEntry>();
      folder->path = entry.path();
      folder->last_modified_at = 0; // TODO implement this

      folder->reload_settings();

      for (const auto& child : std::filesystem::directory_iterator(file_path)) {
        if (child.is_directory() || child.is_symlink()) {
          continue;
        }

        auto codec = sail::codec_info::from_path(child.path());
        if (codec.is_valid()) {
          auto file = std::make_shared<ImageEntry>();
          file->path = child.path();
          file->last_modified_at = 0; // TODO implement this
          file->parent = folder;

          all_entries.push_back(file);
          folder->children.push_back(file);
        }
      }

      all_entries.push_back(folder);
    } else {
      auto codec = sail::codec_info::from_path(file_path);
      if (!codec.is_valid()) {
        continue;
      }

      auto parent_path = entry.path().parent_path();
      auto parent = tmp_folder_entries[parent_path.string()];
      if (parent == nullptr) {
        parent = std::make_shared<FolderEntry>();
        parent->path = parent_path;
        parent->last_modified_at = 0; // TODO implement this

        parent->reload_settings();

        tmp_folder_entries[parent_path.string()] = parent;
        all_entries.push_back(parent);
      }

      auto file = std::make_shared<ImageEntry>();
      file->path = entry.path();
      file->last_modified_at = 0; // TODO implement this
      file->parent = parent;
      parent->children.push_back(file);
      all_entries.push_back(file);
    }
  }

  auto t1 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
  auto duration_ms = static_cast<long long int>(duration.count());

  set_sort_order(options.sort_order);

  refresh_shown_entries();

  log_debug("Loaded %lu images from %lu entries in %lu files in %lld ms", all_entries.size(), shown_entries.size(), file_paths.size(), duration_ms);
}

std::shared_ptr<ImageEntry> Playlist::get_current() const {
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

std::shared_ptr<ImageEntry> Playlist::advance(int by) {
  idx += by;
  if (idx < 0) {
    idx = count + by;
  } else {
    idx = idx % count;
  }
  return get_current();
}

std::shared_ptr<ImageEntry> Playlist::go_to_first() {
  idx = 0;
  return get_current();
}

std::shared_ptr<ImageEntry> Playlist::go_to_last() {
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

void Playlist::current_toggle_favorite() {
  auto current = get_current();
  if (current == nullptr || current->parent == nullptr) {
    return;
  }

  current->parent->toggle_favorite(current->path.filename());

  refresh_shown_entries();
}

void Playlist::current_toggle_hidden() {
  auto current = get_current();
  if (current == nullptr || current->parent == nullptr) {
    return;
  }

  current->parent->toggle_hidden(current->path.filename());

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
    if (entry->is_folder()) {
      continue;
    }

    auto image_entry = std::static_pointer_cast<ImageEntry>(entry);

    if (options.only_favorites && !image_entry->is_favorite()) {
      continue;
    }

    if (options.skip_hidden && image_entry->is_hidden()) {
      continue;
    }

    shown_entries.push_back(image_entry);

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
