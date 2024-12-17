#if defined(WIN32)

#include "platform.h"

using namespace monokl;

Platform::Platform() {
  action_mappings = {
    ActionMapping(SDLK_LEFT, ActionType::GoToPrevious),
    ActionMapping(SDLK_RIGHT, ActionType::GoToNext),
    ActionMapping(SDLK_HOME, ActionType::GoToFirst),
    ActionMapping(SDLK_END, ActionType::GoToLast),
    ActionMapping(SDLK_KP_0, ActionType::FitImageToScreen),
    ActionMapping(SDLK_KP_1, ActionType::ResetZoom),
    ActionMapping(SDLK_f, ActionType::ToggleFavorite),

    ActionMapping(SDLK_f, KMOD_SHIFT, ActionType::ToggleFavoritesOnly),

    ActionMapping(SDLK_f, KMOD_CTRL, ActionType::MaximizeWindow),
    ActionMapping(SDLK_m, KMOD_CTRL, ActionType::MinimizeWindow),
    ActionMapping(SDLK_n, KMOD_CTRL, ActionType::OpenNewWindow),
  };

  user_home_dir = std::filesystem::path(std::getenv("USERPROFILE"));

  window_flags = SDL_WINDOW_METAL;
}

Platform::~Platform() {}

#endif
