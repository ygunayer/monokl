#ifndef MONOKL__WINDOW_H
#define MONOKL__WINDOW_H

#include <sail-c++/image.h>
#include <string>
#include <vector>
#include <memory>

#include <fmt/format.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_hints.h>
#include <SDL_surface.h>

#include "logging.h"
#include "error.h"
#include "playlist.h"

namespace monokl {

class Application;

struct WindowOptions {
  int x = 0;
  int y = 0;
  int width = 1366;
  int height = 768;
  bool centered = true;
  bool maximized = false;

  WindowOptions();
  WindowOptions(const WindowOptions& options);
};

class Window : public std::enable_shared_from_this<Window> {

public:
  ~Window();

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

  explicit Window(const Application& app, const WindowOptions& options);

  const Application& app;

  WindowOptions options;
  std::shared_ptr<Playlist> playlist = nullptr;

  bool is_dropping_files = false;
  std::vector<std::string> dropped_files;
  void begin_drop_files();
  void drop_file(const char* file);
  void end_drop_files();

  SDL_Rect window_rect;
  SDL_Rect image_rect;
  SDL_Rect render_rect;
  double zoom_level = 1.0;
  void recalculate_render_rect();
  void fit_image_to_screen();
  void set_original_image_size();
  void change_zoom(float by);

  unsigned int id = 0;
  bool has_focus = false;
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_Texture* main_tex = nullptr;

  std::unique_ptr<sail::image> current_image = nullptr;
};

}

#endif
