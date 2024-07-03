#ifndef MONOKL__UTIL_H
#define MONOKL__UTIL_H

#include <filesystem>

namespace monokl {

class Util {
public:
  static std::filesystem::path get_user_home_dir() {
  #ifdef _WIN32
    return std::filesystem::path(std::getenv("USERPROFILE"));
  #elif defined(__APPLE__)
    return std::filesystem::path(std::getenv("HOME"));
  #elif defined(__linux__)
    return std::filesystem::path(std::getenv("HOME"));
  #endif
  }
};

};

#endif
