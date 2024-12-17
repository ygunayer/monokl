#pragma once

#include <vector>
#include <filesystem>
#include <clocale>

#include <SDL2/SDL_video.h>

#include "action.h"
#include "application.h"

namespace monokl {

struct Platform {
  virtual void run_main_loop();

  std::vector<ActionMapping> action_mappings;
  std::filesystem::path user_home_dir;
  uint32_t window_flags;

  static std::unique_ptr<Platform> create();
protected:
  std::unique_ptr<Application> app;
  // TODO:
  // - setup_menu() -> set up the menu bar
};

};
