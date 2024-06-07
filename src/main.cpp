#include <fmt/core.h>

int main(int argc, char* argv[]) {
  fmt::println("Hello, {some_arg}", fmt::arg("some_arg", "world!"));
  return 0;
}
