#ifndef ZENNUBE_PLAYLIST_H
#define ZENNUBE_PLAYLIST_H

#include <string>
#include <vector>

namespace zennube {

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
  bool operator()(const PlaylistEntry& a, const PlaylistEntry& b) const;
};

class Playlist {
public:
  Playlist();

  void set_sort_order(const PlaylistSortOrder& sort_order);

  std::vector<PlaylistEntry> entries;
  PlaylistOptions options;
};

}

#endif
