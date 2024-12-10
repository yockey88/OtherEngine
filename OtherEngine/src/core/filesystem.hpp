/**
 * \file core/filesystem.hpp
 **/
#ifndef OTHER_ENGINE_FILESYSTEM_HPP
#define OTHER_ENGINE_FILESYSTEM_HPP

#include <string>
#include <string_view>
#include <vector>

#include "core/config.hpp"
#include "core/defines.hpp"
#include "core/directory.hpp"
#include "core/file_handle.hpp"
#include "core/ref.hpp"

#include "parsing/cmd_line_parser.hpp"

namespace other {

  /// TODO: make this be the install directory at buildtime
  constexpr static std::string_view kEngineCoreDir = "C:/Yock/code/OtherEngine";

  class Filesystem {
   public:
    static void Initialize(const CmdLine& cmdline, const ConfigTable& config);

    static void Poll();

    static bool FileExists(const Path& path);
    static bool PathExists(const Path& path);
    static bool IsDirectory(const Path& path);
    static bool CreateDir(const Path& path);
    static bool AttemptDelete(const Path& path);

    static Ref<Directory> MountProjectRoot(const std::string_view name, const Path& path);
    static Ref<Directory> MountDirectory(const std::string_view name, const Path& path);
    static Ref<FileHandle> RegisterFile(const Path& path);

    static Ref<FileHandle> CreateMemoryFile(const std::string_view drive, const std::string_view virtual_filename, const std::string_view ext);

    static Ref<Directory> OpenDirectory(const Path& path);
    static Ref<FileHandle> OpenFile(const Path& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

    static Ref<Directory> GetDirectory(const std::string_view name);
    static Ref<Directory> GetDirectory(UUID id);

    static Ref<FileHandle> GetFile(const Path& path);
    static Ref<FileHandle> GetFile(UUID id);

    static Ref<Directory> ProjectDirectory();

    static Path GetEngineCoreDir();
    static Path GetWorkingDirectory();
    static Path FindExecutableIn(const Path& path);

    static Path FindCoreFile(const Path& path);
    static Path FindEngineCoreDir(const Path& path);
    static std::vector<Path> GetSubPaths(const Path& path);
    static std::vector<Path> GetSubDirs(const Path& path);
    static std::vector<Path> GetDirectoryFiles(const Path& path);
    static std::string ReadFile(const Path& path);
    static std::vector<char> ReadFileAsChars(const Path& path);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_FILESYSTEM_HPP
