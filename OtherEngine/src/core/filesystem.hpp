/**
 * \file core/filesystem.hpp
 **/
#ifndef OTHER_ENGINE_FILESYSTEM_HPP
#define OTHER_ENGINE_FILESYSTEM_HPP

#include <string>
#include <string_view>
#include <vector>

#include "defines.hpp"

namespace other {
  
  constexpr static std::string_view kEngineCoreDir = "C:/Yock/code/OtherEngine";

  class Filesystem {
    public:
      static bool FileExists(const std::string& path);
      static bool FileExists(const std::string_view path);
      static bool FileExists(const Path& path);
      
      static bool PathExists(const std::string& path);
      static bool PathExists(const std::string_view path);
      static bool PathExists(const Path& path);

      static bool IsDirectory(const std::string& path);
      static bool IsDirectory(const std::string_view path);
      static bool IsDirectory(const Path& path);

      static bool CreateDir(const std::string& path);
      static bool CreateDir(const std::string_view path);
      static bool CreateDir(const Path& path);

      static Path FindExecutableIn(const std::string& path);
      static Path FindExecutableIn(const std::string_view path);
      static Path FindExecutableIn(const Path& path);

      static Path GetWorkingDirectory();

      static Path GetEngineCoreDir();

      static Path FindCoreFile(const std::string& path);
      static Path FindCoreFile(const std::string_view path);
      static Path FindCoreFile(const Path& path);

      static Path FindEngineCoreDir(const std::string& path);
      static Path FindEngineCoreDir(const std::string_view path);
      static Path FindEngineCoreDir(const Path& path);

      static std::vector<Path> GetSubPaths(const std::string& path);
      static std::vector<Path> GetSubPaths(const std::string_view path);
      static std::vector<Path> GetSubPaths(const Path& path);

      static std::vector<Path> GetSubDirs(const std::string& path);
      static std::vector<Path> GetSubDirs(const std::string_view path);
      static std::vector<Path> GetSubDirs(const Path& path);

      static std::vector<Path> GetDirectoryFiles(const std::string& path);
      static std::vector<Path> GetDirectoryFiles(const std::string_view path);
      static std::vector<Path> GetDirectoryFiles(const Path& path);

      static std::string ReadFile(const std::string& path);
      static std::string ReadFile(const std::string_view path);
      static std::string ReadFile(const Path& path);

      static std::vector<char> ReadFileAsChars(const std::string& path);
      static std::vector<char> ReadFileAsChars(const std::string_view path);
      static std::vector<char> ReadFileAsChars(const Path& path);
  };

} // namespace other

#endif // !OTHER_ENGINE_FILESYSTEM_HPP
