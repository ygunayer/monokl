#ifndef MONOKL__UTIL_H
#define MONOKL__UTIL_H

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
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
  }

  static bool is_valid_image(const std::filesystem::path& path) {
    auto path_str = ws2s(path.wstring());
    auto codec = sail::codec_info::from_path(path_str);
    return codec.is_valid();
  }
};

};

#endif
