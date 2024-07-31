/**
 * \file core/filesystem.hpp
 **/
#include "core/filesystem.hpp"

#include <filesystem>
#include <fstream>

#include "core/logger.hpp"

namespace other {

  bool Filesystem::FileExists(const std::string& path) {
    return FileExists(Path(path));
  }

  bool Filesystem::FileExists(const std::string_view path) {
    return FileExists(Path(path));
  }

  bool Filesystem::FileExists(const Path& path) {
    return std::filesystem::exists(path) && !IsDirectory(path);
  }

  bool Filesystem::PathExists(const std::string& path) {
    return PathExists(Path(path));
  }

  bool Filesystem::PathExists(const std::string_view path) {
    return PathExists(Path(path));
  }

  bool Filesystem::PathExists(const Path& path) {
    return std::filesystem::exists(path);
  }

  bool Filesystem::IsDirectory(const std::string& path) {
    return IsDirectory(Path(path));
  }

  bool Filesystem::IsDirectory(const std::string_view path) {
    return IsDirectory(Path(path));
  }

  bool Filesystem::IsDirectory(const Path& path) {
    return std::filesystem::is_directory(path);
  }

  bool Filesystem::CreateDir(const std::string& path) {
    return CreateDir(Path(path));
  }

  bool Filesystem::CreateDir(const std::string_view path) {
    return CreateDir(Path(path));
  }

  bool Filesystem::CreateDir(const Path& path) {
    if (PathExists(path)) {
      return true;
    }
    return std::filesystem::create_directory(path);
  }

  Path Filesystem::FindExecutableIn(const std::string& path) {
    return FindExecutableIn(Path(path));
  }

  Path Filesystem::FindExecutableIn(const std::string_view path) {
    return FindExecutableIn(Path(path));
  }

  Path Filesystem::FindExecutableIn(const Path& path) {
    OE_DEBUG("Attempting to find executable in : {}" , path);
    if (!std::filesystem::exists(path)) {
      OE_WARN("Path does not exist : {}", path.string());
      return Path();
    }

    if (std::filesystem::is_regular_file(path)) {
      if (path.extension() == ".exe") {
        OE_DEBUG("{} is already an exe!" , path);
        return path;
      }

      Path parent = path.parent_path();
      if (parent.empty()) {
        OE_WARN("Parent path is empty : {}", path.string());
        OE_WARN("Could not find executable!");
        return Path();
      }

      return FindExecutableIn(parent);
    }

    /// TODO: fix this so user's config is not bound to engine's config
    Path bin_path = path / "bin";
#ifdef OE_DEBUG_BUILD
    bin_path /= "Debug";
#elif defined(OE_RELEASE_BUILD)
    bin_path /= "Release";
#endif

    OE_DEBUG("Attempting find exe in {}" , bin_path);

    if (!std::filesystem::exists(bin_path)) {
      OE_WARN("Bin path does not exist : {}", bin_path.string());
      return Path();
    }

    std::vector<Path> paths;
    for (const auto& entry : std::filesystem::directory_iterator(bin_path)) {
      if (entry.is_regular_file() && entry.path().extension() == ".exe") {
        paths.push_back(entry.path());
      } 
    }

    if (paths.size() >= 1) {
      if (paths.size() > 1) {
        OE_WARN("Found more than one executable in: {} | Launching the first found : {}", path.string() , paths[0].string());
      }
      return paths[0];
    }

    OE_WARN("Could not find executable in path : {}", bin_path);
    return Path();
  }
      
  bool Filesystem::AttemptDelete(const std::string& path) {
    return AttemptDelete(Path{ path });
  }

  bool Filesystem::AttemptDelete(const std::string_view path) {
    return AttemptDelete(Path{ path });
  }

  bool Filesystem::AttemptDelete(const Path& path) {
    return std::filesystem::remove(path);
  }

  Path Filesystem::GetWorkingDirectory() {
    return std::filesystem::current_path();
  }

  Path Filesystem::GetEngineCoreDir() {
    return kEngineCoreDir;
  }

  Path Filesystem::FindCoreFile(const std::string& path) {
    return FindCoreFile(Path(path));
  }

  Path Filesystem::FindCoreFile(const std::string_view path) {
    return FindCoreFile(Path(path));
  }

  Path Filesystem::FindCoreFile(const Path& path) {
    Path core_dir = kEngineCoreDir;
    auto p = std::filesystem::absolute(core_dir / path);
    OE_DEBUG("Looking for file : {}", p.string());

    if (std::filesystem::exists(p)) {
      return core_dir / path;
    } else {
      // attempt to find it
      OE_DEBUG("Attempting to find file : {}", p.string());
      for (auto& entry : std::filesystem::recursive_directory_iterator(core_dir)) {
        if (entry.is_regular_file()) {
          if (entry.path().filename() == path.filename()) {
            return entry.path();
          }
        }
      }
    }

    return Path();
  }

  Path Filesystem::FindEngineCoreDir(const std::string& path) {
    return FindEngineCoreDir(Path(path));
  }

  Path Filesystem::FindEngineCoreDir(const std::string_view path) {
    return FindEngineCoreDir(Path(path));
  }

  Path Filesystem::FindEngineCoreDir(const Path& path) {
    Path core_dir = kEngineCoreDir;
    if (std::filesystem::exists(core_dir / path)) {
      return core_dir / path;
    } else {
      OE_WARN("Engine core does not conatin path : {}", path);
    }

    return Path();
  }

  std::vector<Path> Filesystem::GetSubPaths(const std::string& path) {
    return GetSubPaths(Path(path));
  }

  std::vector<Path> Filesystem::GetSubPaths(const std::string_view path) {
    return GetSubPaths(Path(path));
  }

  std::vector<Path> Filesystem::GetSubPaths(const Path& path) {
    if (!PathExists(path)) {
      return {};
    }

    std::vector<Path> paths;
    for (auto& entry : std::filesystem::directory_iterator(path)) {
      paths.push_back(entry.path());
    }

    return paths;
  }

  std::vector<Path> Filesystem::GetSubDirs(const std::string& path) {
    return GetSubDirs(Path(path));
  }

  std::vector<Path> Filesystem::GetSubDirs(const std::string_view path) {
    return GetSubDirs(Path(path));
  }

  std::vector<Path> Filesystem::GetSubDirs(const Path& path) {
    if (!PathExists(path)) {
      return {};
    }

    std::vector<Path> paths;
    for (auto& entry : std::filesystem::directory_iterator(path)) {
      if (!entry.is_directory()) {
        continue;
      }
      paths.push_back(entry.path());
    }

    return paths;
  }

  std::vector<Path> Filesystem::GetDirectoryFiles(const std::string& path) {
    return GetDirectoryFiles(Path(path));
  }

  std::vector<Path> Filesystem::GetDirectoryFiles(const std::string_view path) {
    return GetDirectoryFiles(Path(path));
  }

  std::vector<Path> Filesystem::GetDirectoryFiles(const Path& path) {
    if (!PathExists(path)) {
      return {};
    }

    std::vector<Path> paths;
    for (auto& entry : std::filesystem::directory_iterator(path)) {
      if (entry.is_directory() || (entry.is_directory() && entry.path().parent_path().filename().string()[0] != '.')) {
        continue;
      }
      paths.push_back(entry.path());
    }

    return paths;
  }

  std::vector<char> Filesystem::ReadFileAsChars(const std::string& path) {
    return ReadFileAsChars(Path(path));
  }
  
  std::vector<char> Filesystem::ReadFileAsChars(const std::string_view path) {
    return ReadFileAsChars(Path(path));
  }
  
  std::vector<char> Filesystem::ReadFileAsChars(const Path& path) {
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

  std::string Filesystem::ReadFile(const std::string &path) {
    return ReadFile(Path(path));
  }
  
  std::string Filesystem::ReadFile(const std::string_view path) {
    return ReadFile(Path(path));
  }

  std::string Filesystem::ReadFile(const Path& path) {
    if (!FileExists(path)) {
      OE_ERROR("Failed to find {}" , path);
      return "";
    }

    std::ifstream f(path);
    if (!f.is_open()) {
      OE_ERROR("Failed to open {}" , path);
      return "";
    }

    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
  }

} // namespace other
