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

  std::shared_ptr<PlaylistEntry> advance(int by);
  std::shared_ptr<PlaylistEntry> go_to_first();
  std::shared_ptr<PlaylistEntry> go_to_last();

  std::vector<std::shared_ptr<PlaylistEntry>> entries;
  PlaylistOptions options;

  std::shared_ptr<PlaylistEntry> get_current_entry() const;
private:
  int idx = -1;
  unsigned int count = 0;
};

}

#endif
