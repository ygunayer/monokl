#include "application.h"
#include <SDL_events.h>
#include <SDL_keycode.h>

using namespace monokl;

Application::Application() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw MonoklError(fmt::format("Failed to initialize SDL: %s", SDL_GetError()));
  }

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  log_debug("SDL initialized");
  sail::log::set_barrier(SailLogLevel::SAIL_LOG_LEVEL_WARNING);
}

Application::~Application() {
  if (window != nullptr) {
    window.reset();
  }

  log_debug("SDL application terminating");
  SDL_Quit();
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
          }
        } break;

        case SDL_WINDOWEVENT: {
          switch (event.window.event) {
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
