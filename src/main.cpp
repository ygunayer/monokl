#include "SDL_hints.h"
#include "SDL_render.h"
#include <numeric>
#include <stdlib.h>
#include <vector>

#include <fmt/core.h>

#include <SDL.h>
#include <SDL_log.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <SDL_events.h>
#include <SDL_stdinc.h>
#include <SDL_config.h>

// TODO: Determine the window flags based on the platform
#ifdef __APPLE__
#define OTHER_WINDOW_FLAGS SDL_WINDOW_METAL
#elif defined(_WIN32)
#define OTHER_WINDOW_FLAGS SDL_WINDOW_VULKAN
#else
#define OTHER_WINDOW_FLAGS SDL_WINDOW_OPENGL
#endif

#define log_debug(...) SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)

void atexit_handler() {
  log_debug("Exiting...");
  SDL_Quit();
}

std::vector<std::string> discoverImages(const std::vector<std::string>& files) {
  std::vector<std::string> images;
  for (const auto& file : files) {
  }
}

int main(int argc, char* argv[]) {
  std::atexit(atexit_handler);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    fmt::println("Failed to initialize SDL: {}", SDL_GetError());
    return -1;
  }

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  log_debug("SDL initialized");

  SDL_Window* wnd = SDL_CreateWindow("zennube", 0, 0, 1366, 768, SDL_WINDOW_RESIZABLE | OTHER_WINDOW_FLAGS);
  if (wnd == nullptr) {
    log_debug("Failed to create window: %s", SDL_GetError());
    return -1;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == nullptr) {
    log_debug("Failed to create renderer: %s", SDL_GetError());
    SDL_DestroyWindow(wnd);
    return -1;
  }
  
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

  SDL_ShowWindow(wnd);

  std::vector<std::string> droppedFiles;
  bool droppingFiles = false;

  bool running = true;
  while (running) {
    SDL_Event event;

    uint32_t currentTime = SDL_GetTicks64();

    SDL_SetRenderDrawColor(renderer, 49, 49, 49, 255);
    SDL_RenderClear(renderer);

    while (running && SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          log_debug("User requested exit");
          running = false;
        } break;

        case SDL_DROPBEGIN: {
          log_debug("Dropping files...");
          droppingFiles = true;
        } break;

        case SDL_DROPFILE: {
          char* filename = event.drop.file;
          droppedFiles.push_back(filename);
          SDL_free(filename);
        } break;

        case SDL_DROPCOMPLETE: {
          auto joinedFiles = std::accumulate(
            std::next(droppedFiles.begin()),
            droppedFiles.end(),
            droppedFiles[0],
            [](const std::string& acc, const std::string& file) {
              return acc + ", " + file;
            }
          );

          log_debug("Dropped files: %s", joinedFiles.c_str());
          droppedFiles.clear();
          droppingFiles = false;
        } break;
      }
    }

    SDL_RenderPresent(renderer);
  }

  if (wnd != nullptr) {
    SDL_DestroyWindow(wnd);
    log_debug("Window destroyed");
  }

  return 0;
}
