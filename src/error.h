#ifndef MONOKL__ERROR_H
#define MONOKL__ERROR_H

#include <string>
#include <fmt/core.h>
#include <exception>

namespace monokl {

#define raise_error(fmt, ...) throw MonoklError(fmt::format(__VA_ARGS__))

class MonoklError : std::exception {
public:
  MonoklError(const std::string& message) : message(message) {
  }

  const char* what() const noexcept override {
    return message.c_str();
  }
private:
  std::string message;
};

}

#endif
