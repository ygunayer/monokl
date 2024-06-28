#ifndef ZENNUBE_ERROR_H
#define ZENNUBE_ERROR_H

#include <string>
#include <fmt/core.h>
#include <exception>

namespace zennube {

#define raise_error(fmt, ...) throw ZennubeError(fmt::format(__VA_ARGS__))

class ZennubeError : std::exception {
public:
  ZennubeError(const std::string& message) : message(message) {
  }

  const char* what() const noexcept override {
    return message.c_str();
  }
private:
  std::string message;
};

}

#endif
