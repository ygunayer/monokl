#ifndef MONOKL__PLAYLIST_H
#define MONOKL__PLAYLIST_H

#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

#include <sail-c++/sail-c++.h>
#include <sail-c++/codec_info.h>
#include <sail-c++/utils.h>
#include <sail-c++/image_input.h>

#include "logging.h"

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
  std::string name;
  std::string path;
  long last_modified_at;
  bool is_favorite = false;
  bool is_hidden = false;
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

  std::shared_ptr<PlaylistEntry> get_current() const;

  unsigned int size() const;
  int current_index() const;

  std::shared_ptr<PlaylistEntry> advance(int by);
  std::shared_ptr<PlaylistEntry> go_to_first();
  std::shared_ptr<PlaylistEntry> go_to_last();

  void refresh_shown_entries();
  void toggle_only_favorites();
  void toggle_skip_hidden();

  std::vector<std::shared_ptr<PlaylistEntry>> all_entries;
  std::vector<std::shared_ptr<PlaylistEntry>> shown_entries;
  PlaylistOptions options;

private:
  int idx = -1;
  unsigned int count = 0;
};

}

#endif
