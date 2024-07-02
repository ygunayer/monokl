#include <exception>

#include "application.h"
#include "window.h"

using namespace monokl;

int main(int argc, char* argv[]) {
  try {
    Application app;

    WindowOptions options;
    options.width = 1366;
    options.height = 768;
    options.centered = true;

    auto window = app.create_main_window(options);

    app.run_main_loop();
  } catch (const MonoklError& e) {
    fmt::print("Failed to initialize application: {}\n", e.what());
    return 1;
  } catch (const std::exception& e) {
    fmt::print("An unexpected error has occurred: {}\n", e.what());
    return 1;
  }
  return 0;
}
