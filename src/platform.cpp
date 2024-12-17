#include "platform.h"

using namespace monokl;

#if defined(__APPLE__)
#include "platforms/macos/macos_platform.h"
#endif

std::unique_ptr<Platform> Platform::create() {
#if defined(__APPLE__)
  return std::make_unique<MacPlatform>();
#elif defined(WIN32)
  return std::make_unique<WinPlatform>();
#elif defined(__linux__)
  return std::make_unique<LinuxPlatform>();
#else
  throw MonoklError("Unsupported platform");
#endif
}

void Platform::run_main_loop() {}
