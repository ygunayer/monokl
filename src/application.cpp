#include "application.h"
#include "logging.h"
#include <SDL_keycode.h>

using namespace monokl;

std::filesystem::path ApplicationSettings::get_settings_path() {
  return Util::get_user_home_dir().append(".monokl").append("settings.toml");
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

  auto favorites = toml::find<std::vector<std::string>>(data, "favorites");
  auto hidden = toml::find<std::vector<std::string>>(data, "hidden");

  for (const auto& favorite : favorites) {
    settings.favorites.insert(favorite);
  }

  for (const auto& hide : hidden) {
    settings.hidden.insert(hide);
  }

  log_debug("Loaded %lu favorites and %lu hidden images", settings.favorites.size(), settings.hidden.size());

  settings.playlist_options.only_favorites = toml::find_or<bool>(data, "playlist.only_favorites", false);
  settings.playlist_options.skip_hidden = toml::find_or<bool>(data, "playlist.skip_hidden", true);
  settings.playlist_options.sort_order = static_cast<PlaylistSortOrder>(toml::find_or<int>(data, "playlist.sort_order", 0));

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
  data["favorites"] = favorites;
  data["hidden"] = hidden;
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

  log_debug("SDL initialized");
}

Application::~Application() {
  if (settings != nullptr) {
    settings->save();
    settings.reset();
  }

  if (window != nullptr) {
    window.reset();
  }

  log_debug("SDL application terminating");
  SDL_Quit();
}

std::shared_ptr<ApplicationSettings> Application::get_settings() const {
  return settings;
}

void Application::run_main_loop() {
  bool running = true;
  while (running) {
    SDL_Event event;

    while (running && SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          log_debug("User requested exit");
          running = false;
        } break;

        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_LEFT:
              window->playlist_advance(-1);
              break;

            case SDLK_RIGHT:
              window->playlist_advance(1);
              break;

            case SDLK_HOME:
              window->playlist_go_to_first();
              break;

            case SDLK_END:
              window->playlist_go_to_last();
              break;
          
            case SDLK_f:
              if (event.key.keysym.mod & KMOD_SHIFT) {
                window->playlist_toggle_only_favorites();
                break;
              }

              auto current_image = window->playlist->get_current();
              if (current_image == nullptr) {
                break;
              }

              if (current_image->is_favorite) {
                current_image->is_favorite = false;
                settings->favorites.erase(current_image->path);
                log_debug("Removed %s from favorites", current_image->path.c_str());
              } else {
                current_image->is_favorite = true;
                settings->favorites.insert(current_image->path);
                log_debug("Added %s to favorites", current_image->path.c_str());
              }

              window->refresh_title(current_image);

              break;
          }
        } break;

        case SDL_WINDOWEVENT: {
          switch (event.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
            case SDL_WINDOWEVENT_RESIZED:
              window->refresh_size();
              break;
          }
        } break;

        case SDL_DROPBEGIN: {
          window->begin_drop_files();
        } break;

        case SDL_DROPFILE: {
          char* filename = event.drop.file;
          window->drop_file(filename);
          SDL_free(filename);
        } break;

        case SDL_DROPCOMPLETE: {
          window->end_drop_files();

          for (auto& entry : window->playlist->all_entries) {
            if (settings->favorites.find(entry->path) != settings->favorites.end()) {
              entry->is_favorite = true;
            }

            if (settings->hidden.find(entry->path) != settings->hidden.end()) {
              entry->is_hidden = true;
            }
          }
        } break;
      }
    }

    if (window != nullptr) {
      window->render();
    }
  }
}

std::shared_ptr<Window> Application::create_main_window(const WindowOptions& options) {
  if (window != nullptr) {
    throw new MonoklError("Application already has a main window.");
  }

  Window* window = new Window(*this, options);
  this->window.reset(window);
  return this->window;
}
