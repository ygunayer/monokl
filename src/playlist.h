#ifndef MONOKL__PLAYLIST_H
#define MONOKL__PLAYLIST_H

#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <set>

#include <sail-c++/sail-c++.h>
#include <sail-c++/codec_info.h>
#include <sail-c++/utils.h>
#include <sail-c++/image_input.h>

#include <toml.hpp>

#include "logging.h"
#include "util.h"

namespace monokl {

typedef enum {
  PlaylistSortOrderNone,
  PlaylistSortOrderName,
  PlaylistSortOrderNameDesc,
  PlaylistSortOrderDate,
  PlaylistSortOrderDateDesc
} PlaylistSortOrder;

struct PlaylistOptions {
  bool only_favorites = false;
  bool skip_hidden = true;
  PlaylistSortOrder sort_order = PlaylistSortOrderNone;
};

struct PlaylistEntry {
  std::filesystem::path path;
  long last_modified_at;

  virtual bool is_folder() const;
};

struct FolderEntry : public PlaylistEntry {
  std::vector<std::shared_ptr<PlaylistEntry>> children;

  std::set<std::string> favorites;
  std::set<std::string> hidden;
  bool settings_changed;

  void toggle_favorite(const std::string& name);
  void toggle_hidden(const std::string& name);

  bool is_folder() const override;
  void reload_settings();
  void save_settings();
};

struct ImageEntry : public PlaylistEntry {
  std::shared_ptr<FolderEntry> parent;

  bool is_folder() const override;
  bool is_favorite() const;
  bool is_hidden() const;
};

struct PlaylistEntryComparator {
  PlaylistSortOrder sort_order;
  PlaylistEntryComparator(const PlaylistSortOrder& sort_order);
  bool operator()(const std::shared_ptr<PlaylistEntry>& a, const std::shared_ptr<PlaylistEntry>& b) const;
};

class Playlist {
public:
  Playlist();

  void set_sort_order(const PlaylistSortOrder& sort_order);
  void reload_images_from(const std::vector<std::string>& file_paths);

  std::shared_ptr<ImageEntry> get_current() const;

  unsigned int size() const;
  int current_index() const;

  std::shared_ptr<ImageEntry> advance(int by);
  std::shared_ptr<ImageEntry> go_to_first();
  std::shared_ptr<ImageEntry> go_to_last();

  void refresh_shown_entries();
  void toggle_only_favorites();
  void toggle_skip_hidden();

  void current_toggle_favorite();
  void current_toggle_hidden();

  std::vector<std::shared_ptr<PlaylistEntry>> all_entries;
  std::vector<std::shared_ptr<ImageEntry>> shown_entries;
  PlaylistOptions options;

private:
  int idx = -1;
  unsigned int count = 0;
};

}

#endif
