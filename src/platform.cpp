#include "platform.h"

using namespace monokl;

#if defined(__APPLE__)
#include "platforms/macos/macos_platform.h"
#elif defined(_WIN32)
#include "platforms/windows/windows_platform.h"
#elif defined(__linux__)
#include "platforms/linux/linux_platform.h"
#endif

std::shared_ptr<Platform> Platform::instance = nullptr;

std::shared_ptr<Platform> Platform::get() {
  if (instance == nullptr) {
#if defined(__APPLE__)
    instance = std::make_shared<MacPlatform>();
#elif defined(_WIN32)
    instance = std::make_shared<WindowsPlatform>();
#elif defined(__linux__)
    instance = std::make_shared<LinuxPlatform>();
#else
    throw MonoklError("Unsupported platform");
#endif
  }

  return instance;
}

void Platform::handle_event(std::shared_ptr<Event> event) {}

void Platform::run_main_loop() {}

PlatformType Platform::get_type() {
  throw MonoklError("Unsupported platform");
}
