#pragma once

#include <sail-c++/image.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include <fmt/format.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_hints.h>

#include <SDL_video.h>
#include <SDL_surface.h>

#include <sail-common/status.h>

#include "application.h"
#include "logging.h"
#include "error.h"
#include "playlist.h"
#include "event.h"

namespace monokl {

class Application;

struct WindowOptions {
  int display_index = 0;
  int x = SDL_WINDOWPOS_UNDEFINED;
  int y = SDL_WINDOWPOS_UNDEFINED;
  int width = 1366;
  int height = 768;
  bool maximized = false;
  uint32_t flags = SDL_WINDOW_RESIZABLE;

  WindowOptions();
  WindowOptions(const WindowOptions& options);
};

class Window : public std::enable_shared_from_this<Window> {

public:
  ~Window();

  uint32_t id = 0;

  void render();

  void refresh_size();
  void refresh_title();
  void refresh_title(const std::shared_ptr<ImageEntry>& entry);

  void reload_current_image();
  void playlist_advance(int by);
  void playlist_go_to_first();
  void playlist_go_to_last();

  void playlist_current_toggle_favorite();
  void playlist_current_toggle_hidden();

  void playlist_toggle_only_favorites();
  void playlist_toggle_skip_hidden();

private:
  friend class Application;

  explicit Window(Application& app, const WindowOptions& options);

  Application& app;

  WindowOptions options;
  std::shared_ptr<Playlist> playlist = nullptr;

  void close();

  SDL_Rect window_rect;
  SDL_Rect image_rect;
  SDL_Rect render_rect;
  double zoom_level = 1.0;
  void recalculate_render_rect();
  void fit_image_to_screen();
  void set_original_image_size();
  void change_zoom(float by);

  void handle_event(std::shared_ptr<Event> event);

  EventBus::CallbackId callback_id = 0l;
  bool maximized = false;
  bool has_focus = false;
  bool needs_redraw = true;
  bool preclose_complete = false;
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_Texture* main_tex = nullptr;

  std::unique_ptr<sail::image> current_image = nullptr;
};

}
