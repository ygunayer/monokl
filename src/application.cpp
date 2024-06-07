#include "application.h"

Application::Application() {
  init();
}

Application::~Application() {
  if (texMain != nullptr) {
    SDL_DestroyTexture(texMain);
  }

  if (renderer != nullptr) {
    SDL_DestroyRenderer(renderer);
  }

  if (window != nullptr) {
    SDL_DestroyWindow(window);
  }

  SDL_Quit();
}
