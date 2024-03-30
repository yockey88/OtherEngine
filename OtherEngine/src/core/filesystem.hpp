/**
 * \file core/filesystem.hpp
 **/
#ifndef OTHER_ENGINE_FILESYSTEM_HPP
#define OTHER_ENGINE_FILESYSTEM_HPP

#include <string>
#include <string_view>
#include <filesystem>

namespace other {
  
  constexpr static std::string_view kEngineCoreDir = "C:/Yock/code/OtherEngine";

  class Filesystem {
    public:
      static bool FileExists(const std::string& path);
      static bool FileExists(const std::string_view path);
      static bool FileExists(const std::filesystem::path& path);

      static bool IsDirectory(const std::string& path);
      static bool IsDirectory(const std::string_view path);
      static bool IsDirectory(const std::filesystem::path& path);

      static bool CreateDir(const std::string& path);
      static bool CreateDir(const std::string_view path);
      static bool CreateDir(const std::filesystem::path& path);

      static std::filesystem::path FindExecutableIn(const std::string& path);
      static std::filesystem::path FindExecutableIn(const std::string_view path);
      static std::filesystem::path FindExecutableIn(const std::filesystem::path& path);

      static std::filesystem::path GetWorkingDirectory();

      static std::filesystem::path FindEngineCoreDir(const std::string& path);
      static std::filesystem::path FindEngineCoreDir(const std::string_view path);
      static std::filesystem::path FindEngineCoreDir(const std::filesystem::path& path);

      static std::vector<char> ReadFileAsChars(const std::string& path);
  };

} // namespace other

#endif // !OTHER_ENGINE_FILESYSTEM_HPP
