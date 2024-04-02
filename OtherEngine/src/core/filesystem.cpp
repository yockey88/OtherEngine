/**
 * \file core/filesystem.hpp
 **/
#include "core/filesystem.hpp"

#include <filesystem>
#include <fstream>

#include "core/logger.hpp"

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

  bool Filesystem::IsDirectory(const std::string& path) {
    return std::filesystem::is_directory(path);
  }

  bool Filesystem::IsDirectory(const std::string_view path) {
    return std::filesystem::is_directory(path);
  }

  bool Filesystem::IsDirectory(const std::filesystem::path& path) {
    return std::filesystem::is_directory(path);
  }

  bool Filesystem::CreateDir(const std::string& path) {
    return std::filesystem::create_directory(path);
  }

  bool Filesystem::CreateDir(const std::string_view path) {
    return std::filesystem::create_directory(path);
  }

  bool Filesystem::CreateDir(const std::filesystem::path& path) {
    return std::filesystem::create_directory(path);
  }

  std::filesystem::path Filesystem::FindExecutableIn(const std::string& path) {
    return FindExecutableIn(std::filesystem::path(path));
  }

  std::filesystem::path Filesystem::FindExecutableIn(const std::string_view path) {
    return FindExecutableIn(std::filesystem::path(path));
  }

  std::filesystem::path Filesystem::FindExecutableIn(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
      OE_WARN("Path does not exist : {}", path.string());
      return std::filesystem::path();
    }

    if (std::filesystem::is_regular_file(path)) {
      if (path.extension() == ".exe") {
        return path;
      }

      std::filesystem::path parent = path.parent_path();
      if (parent.empty()) {
        OE_WARN("Parent path is empty : {}", path.string());
        return std::filesystem::path();
      }

      return FindExecutableIn(parent);
    }

    /// TODO: fix this so user's config is not bound to engine's config
    std::filesystem::path bin_path = path / "bin";
#ifdef OE_DEBUG_BUILD
    bin_path /= "Debug";
#elif defined(OE_RELEASE_BUILD)
    bin_path /= "Release";
#endif

    if (!std::filesystem::exists(bin_path)) {
      OE_WARN("Bin path does not exist : {}", bin_path.string());
      return std::filesystem::path();
    }

    std::vector<std::filesystem::path> paths;
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
    return std::filesystem::path();
  }

  std::filesystem::path Filesystem::GetWorkingDirectory() {
    return std::filesystem::current_path();
  }

  std::filesystem::path Filesystem::GetEngineCoreDir() {
    return kEngineCoreDir;
  }

  std::filesystem::path Filesystem::FindCoreFile(const std::string& path) {
    return FindCoreFile(std::filesystem::path(path));
  }

  std::filesystem::path Filesystem::FindCoreFile(const std::string_view path) {
    return FindCoreFile(std::filesystem::path(path));
  }

  std::filesystem::path Filesystem::FindCoreFile(const std::filesystem::path& path) {
    std::filesystem::path core_dir = kEngineCoreDir;
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

    return std::filesystem::path();
  }

  std::filesystem::path Filesystem::FindEngineCoreDir(const std::string& path) {
    return FindEngineCoreDir(std::filesystem::path(path));
  }

  std::filesystem::path Filesystem::FindEngineCoreDir(const std::string_view path) {
    return FindEngineCoreDir(std::filesystem::path(path));
  }

  std::filesystem::path Filesystem::FindEngineCoreDir(const std::filesystem::path& path) {
    std::filesystem::path core_dir = kEngineCoreDir;
    if (std::filesystem::exists(core_dir / path)) {
      return core_dir / path;
    } else {
      OE_WARN("Engine core does not conatin path : {}", path);
    }

    return std::filesystem::path();
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
