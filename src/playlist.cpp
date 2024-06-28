#include "playlist.h"

using namespace zennube;

PlaylistEntryComparator::PlaylistEntryComparator(const PlaylistSortOrder& sort_order)
  : sort_order(sort_order) {
}

bool PlaylistEntryComparator::operator()(const PlaylistEntry& a, const PlaylistEntry& b) const {
  switch (sort_order) {
    case PlaylistSortOrderName:
      return a.name < b.name;
    case PlaylistSortOrderNameDesc:
      return a.name > b.name;
    case PlaylistSortOrderDate:
      return a.last_modified_at < b.last_modified_at;
    case PlaylistSortOrderDateDesc:
      return a.last_modified_at > b.last_modified_at;
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