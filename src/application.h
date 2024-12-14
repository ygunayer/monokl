#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include <set>
#include <map>

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

  std::shared_ptr<Window> create_window(const WindowOptions& options);
  std::shared_ptr<ApplicationSettings> get_settings() const;
  std::shared_ptr<Window> get_window_by_id(uint32_t id) const;

private:
  unsigned int focused_window_id = 0;
  bool is_first_window = true;
  int next_window_x = 0;
  int next_window_y = 0;
  std::map<uint32_t, std::shared_ptr<Window>> windows;
  std::shared_ptr<ApplicationSettings> settings;
};

}
