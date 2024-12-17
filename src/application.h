#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include <set>
#include <map>
#include <clocale>

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

#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_scancode.h>

#include <sail-c++/log.h>
#include <sail-common/log.h>

#include <toml.hpp>

#include "playlist.h"
#include "logging.h"
#include "error.h"
#include "window.h"
#include "util.h"
#include "event.h"

namespace monokl {

class Window;
struct WindowOptions;

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

  std::shared_ptr<ApplicationSettings> get_settings() const;
  void create_window(const WindowOptions& options);
  void close_window(unsigned int window_id);
  std::shared_ptr<EventBus> get_event_bus();

private:
  unsigned int focused_window_id = 0;
  bool is_first_window = true;
  int next_window_x = 0;
  int next_window_y = 0;
  std::map<unsigned int, std::unique_ptr<Window>> windows;
  std::shared_ptr<ApplicationSettings> settings;
  std::shared_ptr<EventBus> event_bus;
};

}
