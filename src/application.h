#ifndef MONOKL__APPLICATION_H
#define MONOKL__APPLICATION_H

#include <memory>
#include <unordered_map>
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
#include <SDL2/SDL_keycode.h>

#include <sail-c++/log.h>
#include <sail-common/log.h>

#include "playlist.h"
#include "logging.h"
#include "error.h"
#include "window.h"

namespace monokl {

class Application {
public:
  Application();
  ~Application();

  void run_main_loop();

  std::shared_ptr<Window> create_main_window(const WindowOptions& options);

private:
  unsigned int focused_window_id = 0;
  std::shared_ptr<Window> window = nullptr;
};

struct PersistentAppState {
  PlaylistOptions playlist_options;
};

}

#endif
