#pragma once

#include <vector>
#include <filesystem>
#include <clocale>

#include <SDL2/SDL_video.h>

#include "action.h"
#include "event.h"
#include "application.h"
#include "error.h"

namespace monokl {

enum class PlatformType {
  MacOS,
  Windows,
  Linux,
};

struct Platform {
  virtual void run_main_loop();
  virtual PlatformType get_type();

  std::vector<ActionMapping> action_mappings;
  std::filesystem::path user_home_dir;
  uint32_t window_flags;

  static std::shared_ptr<Platform> get();
protected:
  std::unique_ptr<Application> app;

  virtual void handle_event(std::shared_ptr<Event> event);

  static std::shared_ptr<Platform> instance;
};

};
