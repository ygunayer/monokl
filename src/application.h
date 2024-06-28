#ifndef ZENNUBE_APPLICATION_H
#define ZENNUBE_APPLICATION_H

#include <vector>
#include <string>
#include <filesystem>

#include <fmt/format.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_hints.h>

#include <sail-c++/sail-c++.h>
#include <sail-c++/codec_info.h>
#include <sail-c++/utils.h>
#include <sail-c++/image_input.h>

#include "playlist.h"
#include "logging.h"
#include "error.h"

namespace zennube {

class Application {
public:
  Application();
  ~Application();

  void run_main_loop();
  void load_playlist_from(const std::vector<std::string>& files);

  Playlist playlist;
  std::vector<std::string> dropped_files;

private:
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_Texture* main_tex = nullptr;
};

struct PersistentAppState {
  PlaylistOptions playlist_options;
};

}

#endif
