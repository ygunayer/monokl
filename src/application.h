#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <memory>

#include "SDL.h"
#include "SDL_log.h"
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_events.h"
#include "SDL_stdinc.h"
#include "SDL_config.h"
#include "SDL_render.h"

class Application {
public:
  Application();
  ~Application();

  void init();
  void run_main_loop();

private:
  void handleEvents();
  void update();

private:
  bool initialized = false;
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texMain;
};

#endif
