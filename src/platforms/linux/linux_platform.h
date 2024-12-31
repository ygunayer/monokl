#if defined(__linux__)

#pragma once

#include <memory>

#include "platform.h"

namespace monokl {

struct LinuxPlatform : public Platform {
  LinuxPlatform();
  ~LinuxPlatform();

  void run_main_loop() override;
  void handle_event(std::shared_ptr<Event> event) override;
  void decorate_window(std::shared_ptr<Window> window);

private:
  uint32_t callback_id;
};

};

#endif
