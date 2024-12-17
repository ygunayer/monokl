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

WindowOptions::WindowOptions(const WindowOptions& options) :
  display_index(options.display_index),
  x(options.x),
  y(options.y),
  width(options.width),
  height(options.height),
  maximized(options.maximized) {}

Window::Window(Application& app, const WindowOptions& options) : app(app), options(options) {
  uint32_t flags = SDL_WINDOW_RESIZABLE | OTHER_WINDOW_FLAGS;

  if (options.maximized) {
    flags |= SDL_WINDOW_MAXIMIZED;
  }

  SDL_Window* wnd = SDL_CreateWindow("monokl", options.x, options.y, options.width, options.height, flags);
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

  callback_id = app.get_event_bus()->subscribe([this](const Event& event) {
    handle_event(event);
  });
}

Window::~Window() {
  if (main_tex != nullptr) {
    SDL_DestroyTexture(main_tex);
    log_debug("[window:%d] Texture destroyed", id);
  }

  if (renderer != nullptr) {
    SDL_DestroyRenderer(renderer);
    log_debug("[window:%d] Renderer destroyed", id);
  }

  if (window != nullptr) {
    SDL_DestroyWindow(window);
    log_debug("[window:%d] Window destroyed", id);
  }
}

void Window::close() {
  app.get_event_bus()->unsubscribe(callback_id);

  playlist->save_settings();
  playlist.reset();

  app.close_window(id);
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

void Window::drop_file(const std::string& file) {
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

void Window::handle_event(const Event& event) {
  if (event.window_id != id) {
    return;
  }

  if (event.type != EventType::ActionEvent) {
    return;
  }

  switch (event.action.type) {
    case ActionType::CloseWindow:
      close();
      break;

    case ActionType::OpenNewWindow: {
      WindowOptions options;

      int x = 0, y = 0;
      int bl = 0, bt = 0, br = 0, bb = 0;
      SDL_GetWindowPosition(window, &x, &y);
      SDL_GetWindowBordersSize(window, &bt, &bl, &bb, &br);

      options.display_index = SDL_GetWindowDisplayIndex(window);
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

      app.create_window(options);
    } break;

    case ActionType::RefreshWindowSize:
      refresh_size();
      break;

    case ActionType::BeginDropFiles:
      begin_drop_files();
      break;

    case ActionType::DropFile:
      drop_file(event.action.text_data);
      break;

    case ActionType::EndDropFiles:
      end_drop_files();
      break;

    case ActionType::GoToNext:
      playlist_advance(1);
      break;

    case ActionType::GoToPrevious:
      playlist_advance(-1);
      break;

    case ActionType::GoToFirst:
      playlist_go_to_first();
      break;

    case ActionType::GoToLast:
      playlist_go_to_last();
      break;

    case ActionType::ZoomIn:
      change_zoom(0.1);
      break;

    case ActionType::ZoomOut:
      change_zoom(-0.1);
      break;

    case ActionType::ResetZoom:
      set_original_image_size();
      break;

    case ActionType::ToggleFavorite:
      playlist_current_toggle_favorite();
      break;

    case ActionType::ToggleFavoritesOnly:
      playlist_toggle_only_favorites();
      break;
  }
}
