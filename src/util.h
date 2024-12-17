#pragma once

#include <filesystem>
#include <string>
#include <codecvt>
#include <filesystem>

#include <sail-c++/codec_info.h>

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

  static std::string ws2s(const std::wstring& wstr) {
    size_t len = wcstombs(nullptr, wstr.c_str(), 0);

    char* buffer = new char[len + 1];
    wcstombs(buffer, wstr.c_str(), len);

    std::string result(buffer, len);

    delete[] buffer;

    return result;
  }

  static bool is_valid_image(const std::filesystem::path& path) {
    auto path_str = ws2s(path.wstring());
    auto codec = sail::codec_info::from_path(path_str);
    return codec.is_valid();
  }
};

};
