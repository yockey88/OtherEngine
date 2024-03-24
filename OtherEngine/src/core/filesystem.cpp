/**
 * \file core/filesystem.hpp
 **/
#include "core/filesystem.hpp"

namespace other {

  bool Filesystem::FileExists(const std::string& path) {
    return std::filesystem::exists(path);
  }

  bool Filesystem::FileExists(const std::string_view path) {
    return std::filesystem::exists(path);
  }

  bool Filesystem::FileExists(const std::filesystem::path& path) {
    return std::filesystem::exists(path);
  }

} // namespace other
