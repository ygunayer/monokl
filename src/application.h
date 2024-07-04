#ifndef MONOKL__APPLICATION_H
#define MONOKL__APPLICATION_H

#include <memory>
#include <string>
#include <filesystem>
#include <set>

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

#include <toml.hpp>

#include "playlist.h"
#include "logging.h"
#include "error.h"
#include "window.h"
#include "util.h"

namespace monokl {

struct ApplicationSettings {
  std::set<std::string> favorites;
  std::set<std::string> hidden;
  PlaylistOptions playlist_options;

  static ApplicationSettings load();
  static std::filesystem::path get_settings_path();
  void save();
};

class Application {
public:
  Application();
  ~Application();

  void run_main_loop();

  std::shared_ptr<Window> create_main_window(const WindowOptions& options);
  std::shared_ptr<ApplicationSettings> get_settings() const;

private:
  unsigned int focused_window_id = 0;
  std::shared_ptr<Window> window = nullptr;
  std::shared_ptr<ApplicationSettings> settings;
};

}

#endif
