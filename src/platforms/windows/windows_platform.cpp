#if defined(_WIN32)

#include "platforms/windows/windows_platform.h"

using namespace monokl;

WindowsPlatform::WindowsPlatform() {
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

  user_home_dir = std::filesystem::path(std::getenv("USERPROFILE"));

  ApplicationSettings settings = ApplicationSettings::load();
  settings.action_mappings = action_mappings;
  app = std::make_unique<Application>(settings);
  callback_id = app->get_event_bus()->subscribe(std::bind(&WindowsPlatform::handle_event, this, std::placeholders::_1));

  WindowOptions options;
  app->create_window(options);
}

WindowsPlatform::~WindowsPlatform() {
  for (const auto& [id, menu] : window_menus) {
    DestroyMenu(menu->menu);
    for (const auto& submenu : menu->submenus) {
      DestroyMenu(submenu);
    }
  }

  window_menus.clear();

  if (app != nullptr) {
    app->get_event_bus()->unsubscribe(callback_id);

    app.reset();
  }
}

void WindowsPlatform::handle_event(std::shared_ptr<Event> event) {
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

void WindowsPlatform::run_main_loop() {
  log_debug("Running main loop");
  app->run_main_loop();
}

void WindowsPlatform::decorate_window(std::shared_ptr<Window> window) {
  if (window == nullptr) {
    return;
  }

  HWND hwnd = get_window_handle(window->id);
  if (hwnd == nullptr) {
    return;
  }

  HMENU menu = CreateMenu();

  HMENU file_menu = CreateMenu();
  MENUITEMINFOW file_menu_item;
  file_menu_item.cbSize = sizeof(MENUITEMINFO);
  file_menu_item.fMask = MIIM_STRING | MIIM_ID;
  file_menu_item.wID = 1;
  file_menu_item.dwTypeData = L"&File";
  file_menu_item.hSubMenu = file_menu;
  InsertMenuItemW(menu, 0, TRUE, &file_menu_item);

  SetMenu(hwnd, menu);

  auto window_menu = std::make_unique<WindowMenu>();
  window_menu->menu = menu;
  window_menu->submenus.push_back(file_menu);
  window_menus[window->id] = std::move(window_menu);
}

HWND WindowsPlatform::get_window_handle(uint32_t window_id) {
  SDL_Window* window = SDL_GetWindowFromID(window_id);
  if (window == nullptr) {
    return nullptr;
  }

  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  SDL_GetWindowWMInfo(window, &info);

  return info.info.win.window;
}

#endif
