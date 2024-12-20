#if defined(_WIN32)

#pragma once

#include <SDL_syswm.h>
#include <Windows.h>

#include "platform.h"

namespace monokl {

struct WindowMenu {
  HMENU menu;
  std::vector<HMENU> submenus;
};

struct WindowsPlatform : public Platform {
  WindowsPlatform();
  ~WindowsPlatform();

  void run_main_loop() override;
  void handle_event(std::shared_ptr<Event> event) override;
  void decorate_window(std::shared_ptr<Window> window);

  HWND get_window_handle(uint32_t window_id);

private:
  EventBus::CallbackId callback_id;

  std::unordered_map<uint32_t, std::unique_ptr<WindowMenu>> window_menus;
};

};

#endif
