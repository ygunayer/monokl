#include <exception>

#include "application.h"

using namespace zennube;

int main(int argc, char* argv[]) {
  try {
    Application app;
    app.run_main_loop();
  } catch (const ZennubeError& e) {
    fmt::print("Failed to initialize application: {}\n", e.what());
    return 1;
  } catch (const std::exception& e) {
    fmt::print("An unexpected error has occurred: {}\n", e.what());
    return 1;
  }
  return 0;
}
