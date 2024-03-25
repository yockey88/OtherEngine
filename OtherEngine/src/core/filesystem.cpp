/**
 * \file core/filesystem.hpp
 **/
#include "core/filesystem.hpp"

#include <fstream>

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

  std::vector<char> Filesystem::ReadFileAsChars(const std::string& path) {
    std::ifstream file(path , std::ios::binary | std::ios::ate);

    if (!file.is_open()) 
        return {};

    std::streampos end = file.tellg();
    file.seekg(0 , std::ios::beg);
    uint32_t size = static_cast<uint32_t>(end - file.tellg());

    if (size == 0) 
        return {};

    std::vector<char> buffer(size);
    file.read(buffer.data() , size);
    file.close();

    return buffer;
  }

} // namespace other
