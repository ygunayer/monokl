#include "window.h"
#include "logging.h"
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
  uint32_t flags = SDL_WINDOW_RESIZABLE | OTHER_WINDOW_FLAGS;

  int x = options.centered ? SDL_WINDOWPOS_CENTERED : options.x;
  int y = options.centered ? SDL_WINDOWPOS_CENTERED : options.y;

  if (options.maximized) {
    flags |= SDL_WINDOW_MAXIMIZED;
  }

  SDL_Window* wnd = SDL_CreateWindow("monokl", x, y, 1366, 768, flags);
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
  for (auto entry : playlist->all_entries) {
    if (!entry->is_folder()) {
      continue;
    }

    auto folder_entry = std::static_pointer_cast<FolderEntry>(entry);
    folder_entry->save_settings();
  }

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
  fit_image_to_screen();
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

void Window::fit_image_to_screen() {
  double aspect_ratio = (double)image_rect.w / (double)image_rect.h;
  double window_aspect_ratio = (double)window_rect.w / (double)window_rect.h;

  if (aspect_ratio > window_aspect_ratio) {
    zoom_level = (double)window_rect.w / (double)image_rect.w;
  } else {
    zoom_level = (double)window_rect.h / (double)image_rect.h;
  }

  recalculate_render_rect();
}

void Window::set_original_image_size() {
  zoom_level = 1.0;
  recalculate_render_rect();
}

void Window::recalculate_render_rect() {
  render_rect.w = image_rect.w * zoom_level;
  render_rect.h = image_rect.h * zoom_level;

  render_rect.x = (window_rect.w - render_rect.w) / 2;
  render_rect.y = (window_rect.h - render_rect.h) / 2;

  refresh_title();
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
  refresh_title(entry);

  if (entry == nullptr) {
    return;
  }

  std::string image_path = entry->path.string();
  sail::image_input input(image_path);
  sail::image image = input.next_frame();

  if (!image.is_valid()) {
    log_error("Failed to load image: %s", image_path.c_str());
    return;
  }

  auto convert_result = image.convert(SAIL_PIXEL_FORMAT_BPP32_RGBA);
  if (convert_result != SAIL_OK) {
    log_error("Failed to convert image to RGBA: %s", image_path.c_str());
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
    log_error("Failed to create surface from image: %s", image_path.c_str());
    return;
  }

  SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
  if (tex == nullptr) {
    log_error("Failed to create texture from surface: %s", image_path.c_str());
    return;
  }

  SDL_FreeSurface(surface);

  main_tex = tex;

  image_rect.w = image.width();
  image_rect.h = image.height();

  fit_image_to_screen();
}

void Window::refresh_title() {
  refresh_title(playlist->get_current());
}

void Window::refresh_title(const std::shared_ptr<ImageEntry>& entry) {
  if (entry == nullptr) {
    SDL_SetWindowTitle(window, "monokl");
  } else {
    int zoom_percentage = (int)(zoom_level * 100);
    auto title = fmt::format("[{}%] {}{}/{} - {}", zoom_percentage, entry->is_favorite() ? "♥" : "", playlist->current_index() + 1, playlist->size(), entry->path.filename().string());
    SDL_SetWindowTitle(window, title.c_str());
  }
}

void Window::change_zoom(float by) {
  zoom_level += by;
  if (zoom_level < 0.1) {
    zoom_level = 0.1;
  } else if (zoom_level > 10.0) {
    zoom_level = 10.0;
  }
  recalculate_render_rect();
}

void Window::playlist_current_toggle_favorite() {
  playlist->current_toggle_favorite();
  refresh_title();
}

void Window::playlist_current_toggle_hidden() {
  playlist->current_toggle_hidden();
  refresh_title();
}

void Window::playlist_toggle_only_favorites() {
  playlist->options.only_favorites = !playlist->options.only_favorites;
  playlist->refresh_shown_entries();
  reload_current_image();
}

void Window::playlist_toggle_skip_hidden() {
  playlist->options.skip_hidden = !playlist->options.skip_hidden;
  playlist->refresh_shown_entries();
  reload_current_image();
}
