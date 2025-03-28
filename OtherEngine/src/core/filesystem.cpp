/**
 * \file core/filesystem.hpp
 **/
#include "core/filesystem.hpp"

#include <filesystem>
#include <fstream>

#include "core/config_keys.hpp"
#include "core/directory.hpp"
#include "core/logger.hpp"
#include "core/memory_file.hpp"

#include "parsing/parser_combinators.hpp"

namespace other {

  ArenaAllocator<Filesystem> Filesystem::allocator;
  Filesystem* Filesystem::instance = nullptr;

  namespace {

    struct VirtualFiletree {
      Ref<Directory> dir = nullptr;
      std::map<UUID, Ref<Directory>> mounted_dirs;
      std::map<UUID, Ref<FileHandle>> registered_files;
      /// other important directories
    };
    static VirtualFiletree sFileTree;

  }  // anonymous namespace

  void Filesystem::Initialize(const CmdLine& cmdline, const ConfigTable& config) {
    OE_ASSERT(instance == nullptr, "Filesystem already initialized");
    instance = allocator.Allocate();

    Opt<Path> cwd = std::nullopt;
    Opt<Arg> arg = cmdline.GetArg("--cwd");
    auto cwd_from_cfg = config.GetVal<std::string>(kProjectSection, "WORKING-DIRECTORY");

    if (arg.has_value()) {
      if (arg->args.size() != 1) {
        OE_ERROR("Invalid number of arguments for --cwd : {}", arg->args.size());
        OE_WARN("Expected 1 argument, got : {}", arg->args.size());
      } else {
        cwd = arg->args[0];
      }
    } else if (cwd_from_cfg.has_value()) {
      cwd = cwd_from_cfg.value();
    }

    if (cwd.has_value()) {
      std::filesystem::current_path(cwd.value());
    }

    MountDirectory("working-directory", std::filesystem::current_path());
    MountDirectory("core-shaders", GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders");
  }

  void Filesystem::Shutdown() {
    OE_ASSERT(instance != nullptr, "Filesystem not initialized");

    sFileTree.registered_files.clear();
    sFileTree.mounted_dirs.clear();
    sFileTree.dir = nullptr;

    allocator.Free(instance);
  }

  std::vector<Path> Filesystem::MountedDirectories() {
    OE_ASSERT(instance != nullptr, "Filesystem instance is null");
    OE_ASSERT(sFileTree.dir != nullptr, "Project root is null");

    std::vector<Path> paths;
    for (auto& [id, dir] : sFileTree.mounted_dirs) {
      paths.push_back(dir->AbsolutePath());
    }

    return paths;
  }

  std::vector<Path> Filesystem::MountedFiles() {
    OE_ASSERT(instance != nullptr, "Filesystem instance is null");
    OE_ASSERT(sFileTree.dir != nullptr, "Project root is null");

    std::vector<Path> paths;
    for (auto& [id, file] : sFileTree.registered_files) {
      paths.push_back(file->AbsolutePath());
    }

    return paths;
  }

  Ref<Directory> Filesystem::MountProjectRoot(const std::string_view name, const Path& path) {
    OE_ASSERT(sFileTree.dir == nullptr, "Project root already mounted");
    uint64_t hash = FNV(name);
    sFileTree.dir = sFileTree.mounted_dirs[hash] = NewRef<Directory>(path, hash);
    OE_ASSERT(sFileTree.dir != nullptr, "Failed to create project directory");
    OE_ASSERT(sFileTree.dir->Exists(), "Project directory does not exist : {}", std::filesystem::current_path().string());

    OE_DEBUG("Filesystem initialized with working directory : [{}]", sFileTree.dir->AbsolutePath());
    return sFileTree.dir;
  }

  void Filesystem::Poll() {
    PROFILE_SECTION("Filesystem--Poll");
    if (sFileTree.dir == nullptr) {
      return;
    }
    /// poll for changes in the virtual file tree

    sFileTree.dir->Poll();

    for (auto& [id, file] : sFileTree.registered_files) {
      OE_ASSERT(file != nullptr, "Registered file is null");
      file->Poll();
    }

    for (auto& [id, dir] : sFileTree.mounted_dirs) {
      OE_ASSERT(dir != nullptr, "Mounted directory is null");
      dir->Poll();
    }
  }

  bool Filesystem::FileExists(const Path& path) {
    return std::filesystem::exists(path) && !IsDirectory(path);
  }

  bool Filesystem::PathExists(const Path& path) {
    try {
      return std::filesystem::exists(path);
    } catch (std::filesystem::filesystem_error& e) {
      OE_ERROR("Filesystem error : {}", e.what());
      return false;
    } catch (...) {
      OE_ERROR("Unknown Filesystem error");
      return false;
    }
  }

  bool Filesystem::IsDirectory(const Path& path) {
    try {
      return std::filesystem::is_directory(path);
    } catch (std::filesystem::filesystem_error& e) {
      OE_ERROR("Filesystem error : {}", e.what());
      return false;
    } catch (...) {
      OE_ERROR("Unknown Filesystem error");
      return false;
    }
  }

  bool Filesystem::IsMounted(const Path& path) {
    if (!PathExists(path)) {
      return false;
    }

    for (auto& [id, dir] : sFileTree.mounted_dirs) {
      if (dir->AbsolutePath() == path || dir->ProjectRelativePath() == path) {
        return true;
      }
    }

    return false;
  }

  bool Filesystem::CreateDir(const Path& path) {
    if (PathExists(path)) {
      return true;
    }

    try {
      return std::filesystem::create_directory(path);
    } catch (std::filesystem::filesystem_error& e) {
      OE_ERROR("Filesystem error : {}", e.what());
      return false;
    } catch (...) {
      OE_ERROR("Unknown Filesystem error");
      return false;
    }
  }

  bool Filesystem::RemoveFile(UUID handle) {
    try {
      bool success = sFileTree.dir->RemoveFile(handle);
      if (success) {
        OE_DEBUG("Removed file : {}", handle);
      }

      for (auto& [id, dir] : sFileTree.mounted_dirs) {
        if (dir->RemoveFile(handle)) {
          success = true;
          OE_DEBUG("Removed file : {}", handle);
        }
      }

      auto find_file = sFileTree.registered_files.find(handle);
      if (find_file != sFileTree.registered_files.end()) {
        auto& file = find_file->second;
        if (file->Exists()) {
          success = file->Remove();
        }

        if (!success && file->Exists()) {
          success = file->Remove();
        }
        sFileTree.registered_files.erase(find_file);
      }

      if (success) {
        OE_DEBUG("Removed file : {}", handle);
      }

      return success;
    } catch (std::filesystem::filesystem_error& e) {
      OE_ERROR("Failed to delete file : {}", e.what());
      return false;
    } catch (...) {
      OE_ERROR("Unknown Filesystem error");
      return false;
    }
  }

  bool Filesystem::RemoveDirectory(UUID handle) {
    // try {
    //   return std::filesystem::remove_all(path);
    // } catch (std::filesystem::filesystem_error& e) {
    //   OE_ERROR("Filesystem error : {}", e.what());
    //   return false;
    // } catch (...) {
    //   OE_ERROR("Unknown Filesystem error");
    //   return false;
    // }
    return false;
  }

  UUID Filesystem::GetPathHandle(const Path& path) {
    auto abs_path = std::filesystem::absolute(path);
    OE_TRACE(" > [Filesystem::Hash({})] : {}", abs_path, FNV(abs_path.string()));
    return FNV(abs_path.string());
  }

  Ref<Directory> Filesystem::MountDirectory(const std::string_view name, const Path& path) {
    OE_DEBUG("Mounting directory : {} at {}", name, path.string());
    if (!PathExists(path)) {
      OE_ERROR("Directory does not exist : {}", path.string());
      return nullptr;
    }

    UUID id = FNV(name);
    auto find_dir = sFileTree.mounted_dirs.find(id);
    if (find_dir != sFileTree.mounted_dirs.end()) {
      OE_DEBUG("Directory already mounted : {}", path.string());
      return find_dir->second;
    }

    Ref<Directory> dir = NewRef<Directory>(path, id);
    sFileTree.mounted_dirs[id] = dir;
    OE_DEBUG(" > Mounted directory : {} at {}", name, path.string());

    OE_ASSERT(dir != nullptr, "Failed to mount directory : {}", path.string());
    return dir;
  }

  Ref<FileHandle> Filesystem::RegisterFile(const Path& path) {
    if (!FileExists(path)) {
      OE_ERROR("Failed to register file : {}", path.string());
      return nullptr;
    }

    UUID hash = FNV(path.string());
    auto find_file = sFileTree.registered_files.find(hash);
    if (find_file != sFileTree.registered_files.end()) {
      return find_file->second;
    }

    Ref<FileHandle> file = NewRef<FileHandle>(path);
    if (file == nullptr) {
      OE_ERROR("Failed to open file : {}", path.string());
      return nullptr;
    }

    if (!file->Exists()) {
      OE_ERROR("File does not exist : {}", path.string());
      return nullptr;
    }

    sFileTree.registered_files[hash] = file;
    return file;
  }

  Ref<FileHandle> Filesystem::CreateMemoryFile(const std::string_view drive, const std::string_view virtual_filename, const std::string_view ext) {
    Ref<FileHandle> file = NewRef<MemoryFile>(virtual_filename, ext);
    if (file == nullptr) {
      OE_ERROR("Failed to create memory file : {}", virtual_filename);
      return nullptr;
    }

    return file;
  }

  Ref<Directory> Filesystem::OpenDirectory(const Path& path) {
    for (auto& [id, dir] : sFileTree.mounted_dirs) {
      if (dir->AbsolutePath() == path || dir->ProjectRelativePath() == path) {
        return dir;
      }
    }

    uint64_t hash = FNV(path.stem().string());
    Ref<Directory> dir = NewRef<Directory>(path, hash);
    if (dir == nullptr) {
      OE_ERROR("Failed to open directory : {}", path.string());
      return nullptr;
    }

    sFileTree.mounted_dirs[dir->handle] = dir;
    OE_DEBUG("Opened directory : {}", path.string());

    return dir;
  }

  Ref<FileHandle> Filesystem::OpenFile(const Path& path, std::ios_base::openmode mode) {
    for (auto& [id, dir] : sFileTree.mounted_dirs) {
      if (dir->Contains(path)) {
        return dir->OpenFile(path, mode);
      }
    }

    UUID hash = FNV(path.filename().string());
    auto find_file = sFileTree.registered_files.find(hash);
    if (find_file != sFileTree.registered_files.end()) {
      auto& file = find_file->second;
      if (file->IsOpen()) {
        file->Close();
      }
      file->Open(mode);

      return file;
    }

    Ref<FileHandle> file = NewRef<FileHandle>(path, mode);
    if (file == nullptr) {
      OE_ERROR("Failed to open file : {}", path.string());
      return nullptr;
    }

    if (!file->Exists()) {
      OE_ERROR("File does not exist : {}", path.string());
      return nullptr;
    }

    if (!file->IsOpen()) {
      OE_WARN("Failed to open file : {}", path.string());
    }

    return file;
  }

  Ref<Directory> Filesystem::GetDirectory(const std::string_view name) {
    UUID id = FNV(name);
    Ref<Directory> d = GetDirectory(id);
    if (d == nullptr) {
      OE_ERROR("Failed to find directory : {}", name);
    }
    return d;
  }

  Ref<Directory> Filesystem::GetDirectory(UUID id) {
    auto find_dir = sFileTree.mounted_dirs.find(id);
    if (find_dir != sFileTree.mounted_dirs.end()) {
      return Ref<Directory>::Clone(find_dir->second);
    }

    OE_ERROR("Failed to find directory with id : {}", id);
    return nullptr;
  }

  Ref<FileHandle> Filesystem::FindFileByName(const std::string_view name, Opt<std::string> ext) {
    OE_ASSERT(instance != nullptr, "Filesystem instance is null");
    OE_ASSERT(sFileTree.dir != nullptr, "Project root is null");

    Ref<FileHandle> file = sFileTree.dir->GetFileHandleByName(name, ext);
    if (file != nullptr) {
      return file;
    }

    for (auto& [id, dir] : sFileTree.mounted_dirs) {
      file = dir->GetFileHandleByName(name, ext);
      if (file != nullptr) {
        return file;
      }
    }

    for (auto& [id, file] : sFileTree.registered_files) {
      if (file->FileName() == name) {
        if (!ext.has_value()) {
          return file;
        } else if (file->Extension() == ext.value()) {
          return file;
        }
      }
    }

    OE_ERROR("Failed to find file : {}", name);
    return nullptr;
  }

  Ref<FileHandle> Filesystem::GetFile(const Path& path) {
    for (auto& [id, dir] : sFileTree.mounted_dirs) {
      if (dir->Contains(path)) {
        OE_TRACE("Found path in mounted directory : {} [ full path = {} ]", path.string(), (*dir) / path);
        return dir->OpenFile(path);
      }
    }

    if (!FileExists(path)) {
      OE_ERROR("Failed to find file : {}", path.string());
      return nullptr;
    }

    UUID hash = FNV(path.string());
    auto find_file = sFileTree.registered_files.find(hash);
    if (find_file != sFileTree.registered_files.end()) {
      return find_file->second;
    }

    Ref<FileHandle> file = NewRef<FileHandle>(path);
    if (file == nullptr) {
      OE_ERROR("Failed to open file : {}", path.string());
      return nullptr;
    }

    if (!file->Exists()) {
      OE_ERROR("File does not exist : {}", path.string());
      return nullptr;
    }

    UUID id = file->handle;
    sFileTree.registered_files[id] = file;
    return file;
  }

  Ref<FileHandle> Filesystem::GetFile(UUID file_id) {
    OE_TRACE("Searching for file with id : {}", file_id);
    auto find_file = sFileTree.registered_files.find(file_id);
    if (find_file != sFileTree.registered_files.end()) {
      return find_file->second;
    }

    for (auto& [id, dir] : sFileTree.mounted_dirs) {
      if (dir->Contains(file_id)) {
        OE_TRACE("  > Found file in mounted directory : [{}]", dir->Name());
        return dir->GetFile(file_id);
      }
    }

    OE_ERROR("Failed to find file with id : {}", file_id);
    return nullptr;
  }

  Ref<Directory> Filesystem::ProjectDirectory() {
    return sFileTree.dir;
  }

  Path Filesystem::GetEngineCoreDir() {
    return kEngineCoreDir;
  }

  Path Filesystem::GetWorkingDirectory() {
    return std::filesystem::current_path();
  }

  Path Filesystem::FindExecutableIn(const Path& path) {
    OE_DEBUG("Attempting to find executable in : {}", path);
    if (!std::filesystem::exists(path)) {
      OE_WARN("Path does not exist : {}", path.string());
      return Path();
    }

    if (std::filesystem::is_regular_file(path)) {
      if (path.extension() == ".exe") {
        OE_DEBUG("{} is already an exe!", path);
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

    OE_DEBUG("Attempting find exe in {}", bin_path);

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
        OE_WARN("Found more than one executable in: {} | Launching the first found : {}", path.string(), paths[0].string());
      }
      return paths[0];
    }

    OE_WARN("Could not find executable in path : {}", bin_path);
    return Path();
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

  Path Filesystem::FindEngineCoreDir(const Path& path) {
    Path core_dir = kEngineCoreDir;
    if (std::filesystem::exists(core_dir / path)) {
      return core_dir / path;
    } else {
      OE_WARN("Engine core does not conatin path : {}", path);
    }

    return Path();
  }

  std::vector<Path> Filesystem::GetSubPaths(const Path& path) {
    if (!PathExists(path)) {
      return {};
    }

    std::vector<Path> paths;
    try {
      for (auto& entry : std::filesystem::directory_iterator(path)) {
        paths.push_back(entry.path());
      }
    } catch (std::filesystem::filesystem_error& e) {
      OE_ERROR("Filesystem error : {}", e.what());
      return {};
    } catch (...) {
      OE_ERROR("Unknown Filesystem error");
      return {};
    }

    return paths;
  }

  std::vector<Path> Filesystem::GetSubDirs(const Path& path) {
    if (!PathExists(path)) {
      return {};
    }

    std::vector<Path> paths;
    try {
      for (auto& entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_directory()) {
          continue;
        }
        paths.push_back(entry.path());
      }
    } catch (std::filesystem::filesystem_error& e) {
      OE_ERROR("Filesystem error : {}", e.what());
      return {};
    } catch (...) {
      OE_ERROR("Unknown Filesystem error");
      return {};
    }

    return paths;
  }

  std::vector<Path> Filesystem::GetDirectoryFiles(const Path& path) {
    if (!PathExists(path)) {
      return {};
    }

    std::vector<Path> paths;
    try {
      for (auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory() || (entry.is_directory() && entry.path().parent_path().filename().string()[0] != '.')) {
          continue;
        }
        paths.push_back(entry.path());
      }
    } catch (std::filesystem::filesystem_error& e) {
      OE_ERROR("Filesystem error : {}", e.what());
      return {};
    } catch (...) {
      OE_ERROR("Unknown Filesystem error");
      return {};
    }

    return paths;
  }

  std::vector<char> Filesystem::ReadFileAsChars(const Path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open())
      return {};

    std::streampos end = file.tellg();
    file.seekg(0, std::ios::beg);
    uint32_t size = static_cast<uint32_t>(end - file.tellg());

    if (size == 0)
      return {};

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
  }

  std::string Filesystem::ReadFile(const Path& path) {
    if (!FileExists(path)) {
      OE_ERROR("Failed to find {}", path);
      return "";
    }

    std::ifstream f(path);
    if (!f.is_open()) {
      OE_ERROR("Failed to open {}", path);
      return "";
    }

    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
  }

}  // namespace other
