#include "application.h"
#include "logging.h"

using namespace monokl;

std::filesystem::path ApplicationSettings::get_settings_path() {
  return Util::get_user_home_dir() / ".monokl" / "settings.toml";
}

ApplicationSettings ApplicationSettings::load() {
  std::setlocale(LC_ALL, "C.UTF-8");

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

  auto result = toml::format(data);
  std::ofstream file(path);
  file << result;
  file.close();

  log_debug("Settings saved to %s", path.string().c_str());
}

Application::Application() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw MonoklError(fmt::format("Failed to initialize SDL: %s", SDL_GetError()));
  }

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  sail::log::set_barrier(SailLogLevel::SAIL_LOG_LEVEL_WARNING);

  settings = std::make_shared<ApplicationSettings>(ApplicationSettings::load());
  event_bus = std::make_shared<EventBus>();

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

  log_debug("SDL application terminating");
  SDL_Quit();
}

std::shared_ptr<ApplicationSettings> Application::get_settings() const {
  return settings;
}

std::shared_ptr<EventBus> Application::get_event_bus() {
  return event_bus;
}

void Application::run_main_loop() {
  std::string os = SDL_GetPlatform();

  int cmd_or_ctrl = (os == "Mac OS X" ? KMOD_GUI : KMOD_CTRL);

  bool running = true;

  while (running) {
    SDL_Event sdl_event;

    while (running && SDL_PollEvent(&sdl_event)) {
      if (windows.empty()) {
        log_debug("No open windows left, exiting");
        running = false;
        break;
      }

      if (sdl_event.type == SDL_WINDOWEVENT) {
        switch (sdl_event.window.event) {
          case SDL_WINDOWEVENT_SIZE_CHANGED:
          case SDL_WINDOWEVENT_RESIZED: {
            event_bus->publish(Event(sdl_event.window.windowID, Action(ActionType::RefreshWindowSize)));
          } break;

          case SDL_WINDOWEVENT_CLOSE: {
            event_bus->publish(Event(sdl_event.window.windowID, Action(ActionType::CloseWindow)));
          } break;

          case SDL_WINDOWEVENT_FOCUS_GAINED: {
            log_debug("Window %d gained focus", sdl_event.window.windowID);
          } break;

          case SDL_WINDOWEVENT_RESTORED: {
            log_debug("Window %d gained restored", sdl_event.window.windowID);
          } break;

          case SDL_WINDOWEVENT_EXPOSED: {
            log_debug("Window %d gained exposed", sdl_event.window.windowID);
          } break;
        }

        continue;
      }

      switch (sdl_event.type) {
        case SDL_KEYDOWN: {
          switch (sdl_event.key.keysym.scancode) {
            case SDL_SCANCODE_N:
                if (sdl_event.key.keysym.mod & cmd_or_ctrl) {
                  event_bus->publish(Event(sdl_event.key.windowID, Action(ActionType::OpenNewWindow)));
                }
              break;

            case SDL_SCANCODE_LEFT:
              event_bus->publish(Event(sdl_event.key.windowID, Action(ActionType::GoToPrevious)));
              break;

            case SDL_SCANCODE_RIGHT:
              event_bus->publish(Event(sdl_event.key.windowID, Action(ActionType::GoToNext)));
              break;

            case SDL_SCANCODE_HOME:
              event_bus->publish(Event(sdl_event.key.windowID, Action(ActionType::GoToFirst)));
              break;

            case SDL_SCANCODE_END:
              event_bus->publish(Event(sdl_event.key.windowID, Action(ActionType::GoToLast)));
              break;

            case SDL_SCANCODE_KP_0:
              event_bus->publish(Event(sdl_event.key.windowID, Action(ActionType::FitImageToScreen)));
              break;

            case SDL_SCANCODE_KP_1:
              event_bus->publish(Event(sdl_event.key.windowID, Action(ActionType::ResetZoom)));
              break;

            case SDL_SCANCODE_F: {
              
              if (sdl_event.key.keysym.mod & KMOD_SHIFT) {
                event_bus->publish(Event(sdl_event.key.windowID, Action(ActionType::ToggleFavoritesOnly)));
              } else {
                event_bus->publish(Event(sdl_event.key.windowID, Action(ActionType::ToggleFavorite)));
              }
            } break;

            default:
              break;
          }
        } break;

        case SDL_MOUSEWHEEL: {
            if (sdl_event.wheel.y > 0) {
              event_bus->publish(Event(sdl_event.wheel.windowID, Action(ActionType::ZoomIn)));
            } else if (sdl_event.wheel.y < 0) {
              event_bus->publish(Event(sdl_event.wheel.windowID, Action(ActionType::ZoomOut)));
            }
          }
          break;

        case SDL_DROPBEGIN: {
          event_bus->publish(Event(sdl_event.drop.windowID, Action(ActionType::BeginDropFiles)));
        } break;

        case SDL_DROPFILE: {
          char* filename = sdl_event.drop.file;
          std::string file(filename);
          SDL_free(filename);

          event_bus->publish(Event(sdl_event.drop.windowID, Action(ActionType::DropFile, file)));
        } break;

        case SDL_DROPCOMPLETE: {
          event_bus->publish(Event(sdl_event.drop.windowID, Action(ActionType::EndDropFiles)));
        } break;
      }
    }

    for (auto& window : windows) {
      window.second->render();
    }
  }
}

void Application::create_window(const WindowOptions& options) {
  auto window = std::unique_ptr<Window>(new Window(*this, options));
  this->windows[window->id] = std::move(window);
}

void Application::close_window(unsigned int window_id) {
  auto it = windows.find(window_id);
  if (it != windows.end()) {
    windows.erase(it);
  }
}
