#include "application.h"
#include "logging.h"
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_scancode.h>

using namespace monokl;

std::filesystem::path ApplicationSettings::get_settings_path() {
  return Util::get_user_home_dir() / ".monokl" / "settings.toml";
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

void Application::run_main_loop() {
  std::string os = SDL_GetPlatform();

  int cmd_or_ctrl = (os == "Mac OS X" ? KMOD_GUI : KMOD_CTRL);

  bool running = true;

  while (running) {
    SDL_Event event;

    while (running && SDL_PollEvent(&event)) {
      if (windows.empty()) {
        log_debug("No open windows left, exiting");
        running = false;
        break;
      }

      if (event.type == SDL_KEYDOWN &&
          event.key.keysym.scancode == SDL_SCANCODE_N &&
          event.key.keysym.mod & cmd_or_ctrl
        ) {
        WindowOptions options;

        auto& window = get_window_by_id(event.key.windowID);
        if (window) {
          int x = 0, y = 0;
          int bl = 0, bt = 0, br = 0, bb = 0;
          SDL_GetWindowPosition(window->window, &x, &y);
          SDL_GetWindowBordersSize(window->window, &bt, &bl, &bb, &br);

          options.display_index = SDL_GetWindowDisplayIndex(window->window);
          SDL_DisplayMode display_mode;
          SDL_GetCurrentDisplayMode(options.display_index, &display_mode);

          int effective_width = display_mode.w - bl - br;
          int effective_height = display_mode.h - bt - bb;

          x += (30 > bl ? 30 : bl);
          y += (30 > bt ? 30 : bt);

          if (x + options.width >= effective_width || y + options.height >= effective_height) {
            x = bl;
            y = bt;
          }

          options.x = x;
          options.y = y;
        }

        create_window(options);
        continue;
      }

      if (event.type == SDL_WINDOWEVENT) {
        switch (event.window.event) {
          case SDL_WINDOWEVENT_SIZE_CHANGED:
          case SDL_WINDOWEVENT_RESIZED: {
            auto& window = get_window_by_id(event.window.windowID);

            if (window == nullptr) {
              continue;
            }

            window->refresh_size();
          } break;

          case SDL_WINDOWEVENT_CLOSE: {
            auto& window = get_window_by_id(event.window.windowID);
            windows.erase(event.window.windowID);

            int num_refs = window.use_count();
            if (num_refs > 1) {
              log_debug("Window %d has %d references", event.window.windowID, num_refs);
            }
          } break;
        }

        continue;
      }

      switch (event.type) {
        case SDL_KEYDOWN: {
          auto& window = get_window_by_id(event.key.windowID);
          if (window == nullptr) {
            break;
          }

          switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_LEFT:
              window->playlist_advance(-1);
              break;

            case SDL_SCANCODE_RIGHT:
              window->playlist_advance(1);
              break;

            case SDL_SCANCODE_HOME:
              window->playlist_go_to_first();
              break;

            case SDL_SCANCODE_END:
              window->playlist_go_to_last();
              break;

            case SDL_SCANCODE_KP_0:
              window->fit_image_to_screen();
              break;

            case SDL_SCANCODE_KP_1:
              window->set_original_image_size();
              break;

            case SDL_SCANCODE_F:
              if (event.key.keysym.mod & KMOD_SHIFT) {
                window->playlist_toggle_only_favorites();
                break;
              }
              window->playlist_current_toggle_favorite();
              break;

            default:
              break;
          }
        } break;

        case SDL_MOUSEWHEEL: {
            auto& window = get_window_by_id(event.wheel.windowID);
            if (window == nullptr) {
              break;
            }

            if (event.wheel.y > 0) {
              window->change_zoom(0.1);
            } else if (event.wheel.y < 0) {
              window->change_zoom(-0.1);
            }
          }
          break;

        case SDL_DROPBEGIN: {
          auto& window = get_window_by_id(event.drop.windowID);
          if (window == nullptr) {
            break;
          }

          window->begin_drop_files();
        } break;

        case SDL_DROPFILE: {
          auto& window = get_window_by_id(event.drop.windowID);
          if (window == nullptr) {
            break;
          }

          char* filename = event.drop.file;
          window->drop_file(filename);
          SDL_free(filename);
        } break;

        case SDL_DROPCOMPLETE: {
          auto& window = get_window_by_id(event.drop.windowID);
          if (window == nullptr) {
            break;
          }

          window->end_drop_files();
        } break;
      }
    }

    for (auto& window : windows) {
      window.second->render();
    }
  }
}

std::shared_ptr<Window> Application::create_window(const WindowOptions& options) {
  auto& window = std::shared_ptr<Window>(new Window(*this, options));
  this->windows[window->id] = window;
  return window;
}

std::shared_ptr<Window> Application::get_window_by_id(uint32_t id) const {
  auto it = windows.find(id);
  if (it != windows.end()) {
    return it->second;
  }

  return nullptr;
}
