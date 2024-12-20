#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include <map>
#include <atomic>

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

  // TODO: this is currently not persisted, but loaded by the platform and injected here
  std::vector<ActionMapping> action_mappings;

  std::vector<std::string> recent_files;

  ApplicationSettings();
  ApplicationSettings(const ApplicationSettings& settings);

  static ApplicationSettings load();
  static std::filesystem::path get_settings_path();
  void save();
};

class Application {
public:
  Application(const ApplicationSettings& settings);
  ~Application();

  void run_main_loop();

  void create_window(const WindowOptions& options);
  void on_window_closed(unsigned int window_id);
  std::shared_ptr<EventBus> get_event_bus();
  std::optional<unsigned int> get_last_window_id();

private:
  bool is_first_window = true;
  std::atomic<bool> running = true;
  int next_window_x = 0;
  int next_window_y = 0;
  std::map<unsigned int, std::vector<std::string>> dropped_files;
  std::map<unsigned int, std::unique_ptr<Window>> windows;
  std::unique_ptr<ApplicationSettings> settings;
  std::shared_ptr<EventBus> event_bus;
  std::vector<unsigned int> windows_pending_cleanup;

  void handle_sdl_event(const SDL_Event& sdl_event);
};

}
