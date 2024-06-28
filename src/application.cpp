#include "application.h"

using namespace zennube;

// TODO: Determine the window flags based on the platform
#ifdef __APPLE__
#define OTHER_WINDOW_FLAGS SDL_WINDOW_METAL
#elif defined(_WIN32)
#define OTHER_WINDOW_FLAGS SDL_WINDOW_VULKAN
#else
#define OTHER_WINDOW_FLAGS SDL_WINDOW_OPENGL
#endif

Application::Application() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw ZennubeError(fmt::format("Failed to initialize SDL: %s", SDL_GetError()));
  }

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  log_debug("SDL initialized");

  SDL_Window* wnd = SDL_CreateWindow("zennube", 0, 0, 1366, 768, SDL_WINDOW_RESIZABLE | OTHER_WINDOW_FLAGS);
  if (wnd == nullptr) {
    throw ZennubeError(fmt::format("Failed to create window: %s", SDL_GetError()));
  }
  window = wnd;

  SDL_Renderer* rnd = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (rnd == nullptr) {
    throw ZennubeError(fmt::format("Failed to create renderer: %s", SDL_GetError()));
  }
  renderer = rnd;

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

  SDL_ShowWindow(wnd);
}

Application::~Application() {
  if (main_tex != nullptr) {
    SDL_DestroyTexture(main_tex);
    log_debug("Texture destroyed");
  }

  if (renderer != nullptr) {
    SDL_DestroyRenderer(renderer);
    log_debug("Renderer destroyed");
  }

  if (window != nullptr) {
    SDL_DestroyWindow(window);
    log_debug("Window destroyed");
  }

  SDL_Quit();
}

void Application::run_main_loop() {
  std::vector<std::string> dropped_files;
  bool is_dropping_files = false;

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
          is_dropping_files = true;
        } break;

        case SDL_DROPFILE: {
          char* filename = event.drop.file;
          dropped_files.push_back(filename);
          SDL_free(filename);
        } break;

        case SDL_DROPCOMPLETE: {
          load_playlist_from(dropped_files);

          dropped_files.clear();
          is_dropping_files = false;
        } break;
      }
    }

    SDL_RenderPresent(renderer);
  }
}

void Application::load_playlist_from(const std::vector<std::string>& files) {
  playlist.entries.clear();

  for (const auto& file_path : files) {
    std::filesystem::directory_entry entry(file_path);

    if (entry.is_directory()) {
      for (const auto& child : std::filesystem::directory_iterator(file_path)) {
        if (child.is_directory() || child.is_symlink()) {
          continue;
        }

        auto codec = sail::codec_info::from_path(child.path());
        if (codec.is_valid()) {
          log_debug("Codec: %s", codec.name().c_str());
          playlist.entries.push_back({
            .name = child.path().filename().string(),
            .path = child.path().string(),
            .last_modified_at = 0,
          });
        }
      }
    } else {
      auto codec = sail::codec_info::from_path(file_path);
      if (codec.is_valid()) {
        std::filesystem::path path(file_path);
        log_debug("Codec: %s", codec.name().c_str());
        playlist.entries.push_back({
          .name = path.filename().string(),
          .path = path.string(),
          .last_modified_at = 0,
        });
      }
    }

    playlist.set_sort_order(PlaylistSortOrderName);

    log_debug("Loaded %lu images from %lu files", playlist.entries.size(), files.size());

    for (const auto& entry : playlist.entries) {
      log_debug("Entry: %s", entry.path.c_str());
    }
  }
}
