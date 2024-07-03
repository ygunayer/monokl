#include "window.h"
#include <SDL_surface.h>
#include <SDL_video.h>
#include <sail-common/status.h>

// TODO: Determine the window flags based on the platform
#ifdef __APPLE__
#define OTHER_WINDOW_FLAGS SDL_WINDOW_METAL
#elif defined(_WIN32)
#define OTHER_WINDOW_FLAGS SDL_WINDOW_OPENGL
#else
#define OTHER_WINDOW_FLAGS SDL_WINDOW_OPENGL
#endif

using namespace monokl;

WindowOptions::WindowOptions() {}

WindowOptions::WindowOptions(const WindowOptions& options) {
  x = options.x;
  y = options.y;
  width = options.width;
  height = options.height;
  centered = options.centered;
  maximized = options.maximized;
}

Window::Window(const Application& app, const WindowOptions& options) : app(app), options(options) {
  SDL_Window* wnd = SDL_CreateWindow("monokl", 0, 0, 1366, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED | OTHER_WINDOW_FLAGS);
  if (wnd == nullptr) {
    throw MonoklError(fmt::format("Failed to create window: %s", SDL_GetError()));
  }
  window = wnd;

  SDL_Renderer* rnd = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (rnd == nullptr) {
    throw MonoklError(fmt::format("Failed to create renderer: %s", SDL_GetError()));
  }
  renderer = rnd;

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

  SDL_ShowWindow(wnd);

  id = SDL_GetWindowID(wnd);
  playlist = std::make_shared<Playlist>();

  refresh_size();
}

Window::~Window() {
  playlist.reset();

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
}

void Window::refresh_size() {
  int old_w = window_rect.w;
  int old_h = window_rect.h;
  SDL_GetWindowSize(window, &window_rect.w, &window_rect.h);

  if (old_w == window_rect.w && old_h == window_rect.h) {
    return;
  }

  fmt::println("Window size updated: {}x{}", window_rect.w, window_rect.h);
  recalculate_render_rect();
}

void Window::render() {
  SDL_SetRenderDrawColor(renderer, 49, 49, 49, 255);
  SDL_RenderClear(renderer);
  if (main_tex != nullptr) {
    SDL_RenderCopy(renderer, main_tex, nullptr, &render_rect);
  }
  SDL_RenderPresent(renderer);
}

void Window::begin_drop_files() {
  log_debug("Dropping files...");
  is_dropping_files = true;
}

void Window::drop_file(const char* file) {
  dropped_files.push_back(file);
}

void Window::end_drop_files() {
  playlist->reload_images_from(dropped_files);
  dropped_files.clear();
  is_dropping_files = false;

  reload_current_image();
}

void Window::playlist_advance(int by) {
  playlist->advance(by);
  reload_current_image();
}

void Window::playlist_go_to_first() {
  playlist->go_to_first();
  reload_current_image();
}

void Window::playlist_go_to_last() {
  playlist->go_to_last();
  reload_current_image();
}

void Window::recalculate_render_rect() {
  double aspect_ratio = (double)image_rect.w / (double)image_rect.h;
  double window_aspect_ratio = (double)window_rect.w / (double)window_rect.h;

  if (aspect_ratio > window_aspect_ratio) {
    render_rect.w = window_rect.w;
    render_rect.h = (int)((double)window_rect.w / aspect_ratio);
  } else {
    render_rect.h = window_rect.h;
    render_rect.w = (int)((double)window_rect.h * aspect_ratio);
  }

  render_rect.x = (window_rect.w - render_rect.w) / 2;
  render_rect.y = (window_rect.h - render_rect.h) / 2;
}

void Window::reload_current_image() {
  if (main_tex != nullptr) {
    SDL_DestroyTexture(main_tex);
    main_tex = nullptr;
  }

  if (current_image != nullptr) {
    current_image.reset();
  }

  image_rect.h = 0;
  image_rect.w = 0;
  image_rect.x = 0;
  image_rect.y = 0;

  auto entry = playlist->get_current();
  if (entry == nullptr) {
    SDL_SetWindowTitle(window, "monokl");
    return;
  }

  std::string title = fmt::format("[{}/{}] {}", playlist->current_index() + 1, playlist->size(), entry->name);
  SDL_SetWindowTitle(window, title.c_str());

  sail::image_input input(entry->path);
  sail::image image = input.next_frame();

  if (!image.is_valid()) {
    log_error("Failed to load image: %s", entry->path.c_str());
    return;
  }

  auto convert_result = image.convert(SAIL_PIXEL_FORMAT_BPP32_RGBA);
  if (convert_result != SAIL_OK) {
    log_error("Failed to convert image to RGBA: %s", entry->path.c_str());
    return;
  }

  SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
    image.pixels(),
    image.width(),
    image.height(),
    32,
    image.bytes_per_line(),
    0x000000FF,
    0x0000FF00,
    0x00FF0000,
    0xFF000000
  );

  if (surface == nullptr) {
    log_error("Failed to create surface from image: %s", entry->path.c_str());
    return;
  }

  SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
  if (tex == nullptr) {
    log_error("Failed to create texture from surface: %s", entry->path.c_str());
    return;
  }

  SDL_FreeSurface(surface);

  main_tex = tex;

  image_rect.w = image.width();
  image_rect.h = image.height();

  recalculate_render_rect();
}
