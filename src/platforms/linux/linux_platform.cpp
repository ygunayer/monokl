#if defined(__linux__)

#include "platforms/linux/linux_platform.h"

using namespace monokl;

LinuxPlatform::LinuxPlatform() {
  std::setlocale(LC_ALL, "C.UTF-8");

  action_mappings = {
    ActionMapping(SDLK_f, KMOD_SHIFT, ActionType::ToggleFavoritesOnly),

    ActionMapping(SDLK_f, KMOD_CTRL, ActionType::MaximizeWindow),
    ActionMapping(SDLK_m, KMOD_CTRL, ActionType::MinimizeWindow),
    ActionMapping(SDLK_n, KMOD_CTRL, ActionType::OpenNewWindow),
    ActionMapping(SDLK_w, KMOD_CTRL, ActionType::CloseWindow),

    ActionMapping(SDLK_LEFT, ActionType::GoToPrevious),
    ActionMapping(SDLK_RIGHT, ActionType::GoToNext),
    ActionMapping(SDLK_HOME, ActionType::GoToFirst),
    ActionMapping(SDLK_END, ActionType::GoToLast),
    ActionMapping(SDLK_KP_0, ActionType::FitImageToScreen),
    ActionMapping(SDLK_KP_1, ActionType::ResetZoom),
    ActionMapping(SDLK_f, ActionType::ToggleFavorite),
  };

  user_home_dir = std::filesystem::path(std::getenv("HOME"));

  ApplicationSettings settings = ApplicationSettings::load();
  settings.action_mappings = action_mappings;
  app = std::make_unique<Application>(settings);
  callback_id = app->get_event_bus()->subscribe(std::bind(&LinuxPlatform::handle_event, this, std::placeholders::_1));

  WindowOptions options;
  app->create_window(options);
}

LinuxPlatform::~LinuxPlatform() {
  if (app != nullptr) {
    app.reset();
  }
}

void LinuxPlatform::handle_event(std::shared_ptr<Event> event) {
  if (event == nullptr) {
    return;
  }

  if (event->type == EventType::WindowEvent) {
    if (event->window == nullptr) {
      return;
    }

    switch (event->window->type) {
      case WindowEventType::Created:
        log_debug("Window maximized");
        break;

      case WindowEventType::Closed:

        break;
    }
  }
}

void LinuxPlatform::run_main_loop() {
  log_debug("Running main loop");
  app->run_main_loop();
}

#endif
