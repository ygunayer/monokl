#include "application.h"
#include "logging.h"
#include <SDL_video.h>
#include <optional>
#include <toml11/types.hpp>

using namespace monokl;

std::filesystem::path ApplicationSettings::get_settings_path() {
  return Util::get_user_home_dir() / ".monokl" / "settings.toml";
}

ApplicationSettings::ApplicationSettings() {}

ApplicationSettings::ApplicationSettings(const ApplicationSettings& settings) {
  this->playlist_options = settings.playlist_options;
  this->action_mappings = settings.action_mappings;
}

ApplicationSettings ApplicationSettings::load() {
  auto path = get_settings_path();

  std::filesystem::directory_entry entry(path);
  if (!entry.exists()) {
    log_debug("No settings found at %s", path.string().c_str());
    return ApplicationSettings();
  }

  auto data = toml::parse(path);

  ApplicationSettings settings;

  if (data.contains("playlist") && data.at("playlist").is_table()) {
    auto playlist_entry = data.at("playlist");

    if (playlist_entry.contains("only_favorites") && playlist_entry.at("only_favorites").is_boolean()) {
      settings.playlist_options.only_favorites = toml::find<bool>(playlist_entry, "only_favorites");
    }

    if (playlist_entry.contains("skip_hidden") && playlist_entry.at("skip_hidden").is_boolean()) {
      settings.playlist_options.skip_hidden = toml::find<bool>(playlist_entry, "skip_hidden");
    }

    if (playlist_entry.contains("sort_order") && playlist_entry.at("sort_order").is_integer()) {
      settings.playlist_options.sort_order = static_cast<PlaylistSortOrder>(toml::find<int>(playlist_entry, "sort_order"));
    }
  }

  if (data.contains("recent_files") && data.at("recent_files").is_array()) {
    auto recent_files = data.at("recent_files").as_array();
    for (auto& file : recent_files) {
      settings.recent_files.push_back(file.as_string());
    }
  }

  log_debug("Loaded settings from %s", path.string().c_str());

  return settings;
}

void ApplicationSettings::save() {
  auto path = get_settings_path();

  std::filesystem::directory_entry entry(path);
  if (!entry.exists()) {
    std::filesystem::create_directories(path.parent_path());
  }

  toml::value data;
  data["playlist"]["only_favorites"] = playlist_options.only_favorites;
  data["playlist"]["skip_hidden"] = playlist_options.skip_hidden;
  data["playlist"]["sort_order"] = static_cast<int>(playlist_options.sort_order);

  toml::array recent_files;
  for (auto& file : recent_files) {
    recent_files.push_back(file);
  }
  data["recent_files"] = recent_files;

  auto result = toml::format(data);
  std::ofstream file(path);
  file << result;
  file.close();

  log_debug("Settings saved to %s", path.string().c_str());
}

Application::Application(const ApplicationSettings& settings) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw MonoklError(fmt::format("Failed to initialize SDL: %s", SDL_GetError()));
  }

  this->settings = std::make_unique<ApplicationSettings>(settings);
  event_bus = std::make_shared<EventBus>();

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  sail::log::set_barrier(SailLogLevel::SAIL_LOG_LEVEL_WARNING);

  log_debug("Application initialized");
  log_debug("Library versions:");
  log_debug(" - SDL2: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
  log_debug(" - Sail: %s", SAIL_VERSION_STRING);
  log_debug(" - fmt: %d.%d.%d", FMT_VERSION / 10000, FMT_VERSION / 100 % 100, FMT_VERSION % 100);
  log_debug(" - toml11: %d.%d.%d", TOML11_VERSION_MAJOR, TOML11_VERSION_MINOR, TOML11_VERSION_PATCH);
}

Application::~Application() {
  if (settings != nullptr) {
    settings->save();
    settings.reset();
  }

  windows.clear();

  if (event_bus != nullptr) {
    event_bus.reset();
  }

  log_debug("SDL application terminating");
  SDL_Quit();
}

std::shared_ptr<EventBus> Application::get_event_bus() {
  return event_bus;
}

void Application::handle_sdl_event(const SDL_Event& sdl_event) {
  if (sdl_event.type == SDL_QUIT) {
    running = false;
  }

  if (sdl_event.type == SDL_WINDOWEVENT) {
    switch (sdl_event.window.event) {
      case SDL_WINDOWEVENT_MAXIMIZED:
        event_bus->publish(std::make_shared<Event>(
          sdl_event.window.windowID,
          std::make_shared<WindowEvent>(WindowEventType::Maximized)
        ));
        break;

      case SDL_WINDOWEVENT_MINIMIZED:
        event_bus->publish(std::make_shared<Event>(
          sdl_event.window.windowID,
          std::make_shared<WindowEvent>(WindowEventType::Minimized)
        ));
        break;

      case SDL_WINDOWEVENT_RESTORED:
        event_bus->publish(std::make_shared<Event>(
          sdl_event.window.windowID,
          std::make_shared<WindowEvent>(WindowEventType::Restored)
        ));
        break;

      case SDL_WINDOWEVENT_SIZE_CHANGED:
      case SDL_WINDOWEVENT_RESIZED: {
        event_bus->publish(std::make_shared<Event>(
          sdl_event.window.windowID,
          std::make_shared<WindowEvent>(WindowEventType::Resized)
        ));
      } break;

      case SDL_WINDOWEVENT_CLOSE: {
        event_bus->publish(std::make_shared<Event>(
          sdl_event.window.windowID,
          std::make_shared<Action>(ActionType::CloseWindow)
        ));
      } break;
    }

    return;
  }

  if (sdl_event.type == SDL_KEYDOWN) {
    for (auto& mapping : settings->action_mappings) {
      if (mapping.matches(sdl_event.key)) {
        event_bus->publish(std::make_shared<Event>(
          sdl_event.key.windowID,
          std::make_shared<Action>(mapping.action)
        ));
        break;
      }
    }
  }

  if (sdl_event.type == SDL_MOUSEWHEEL) {
    if (sdl_event.wheel.y > 0) {
      event_bus->publish(std::make_shared<Event>(
        sdl_event.wheel.windowID,
        std::make_shared<Action>(ActionType::ZoomIn)
      ));
    } else if (sdl_event.wheel.y < 0) {
      event_bus->publish(std::make_shared<Event>(
        sdl_event.wheel.windowID,
        std::make_shared<Action>(ActionType::ZoomOut)
      ));
    }
  }

  if (sdl_event.type == SDL_DROPBEGIN) {
    dropped_files[sdl_event.drop.windowID] = std::vector<std::string>();
  }

  if (sdl_event.type == SDL_DROPFILE) {
    char* filename = sdl_event.drop.file;
    std::string file(filename);
    SDL_free(filename);

    if (dropped_files.find(sdl_event.drop.windowID) != dropped_files.end()) {
      dropped_files[sdl_event.drop.windowID].push_back(file);
    }
  }

  if (sdl_event.type == SDL_DROPCOMPLETE) {
    if (dropped_files.find(sdl_event.drop.windowID) != dropped_files.end()) {
      auto files = dropped_files[sdl_event.drop.windowID];
      event_bus->publish(std::make_shared<Event>(
        sdl_event.drop.windowID,
        std::make_shared<OpenFilesAction>(files)
      ));
      dropped_files.erase(sdl_event.drop.windowID);
    }
  }
}

void Application::run_main_loop() {
  while (running) {
    SDL_Event sdl_event;
    int has_event = SDL_PollEvent(&sdl_event);

    if (windows_pending_cleanup.size() > 0) {
      for (auto& window_id : windows_pending_cleanup) {
        windows.erase(window_id);
      }
      windows_pending_cleanup.clear();
    }

    if (windows.empty()) {
      log_debug("No open windows left, exiting");
      running = false;
      break;
    }

    if (has_event) {
      handle_sdl_event(sdl_event);
    }

    for (auto& window : windows) {
      window.second->render();
    }
  }
}

void Application::create_window(const WindowOptions& options) {
  auto window = std::unique_ptr<Window>(new Window(*this, options));
  windows[window->id] = std::move(window);
}

void Application::on_window_closed(unsigned int window_id) {
  windows_pending_cleanup.push_back(window_id);
}

std::optional<unsigned int> Application::get_last_window_id() {
  if (windows.empty()) {
    return std::nullopt;
  }

  return windows.rbegin()->first;
}
