/**
 * \file core/directory.cpp
 **/
#include "core/directory.hpp"

#include <filesystem>
#include <ranges>

#include "core/logger.hpp"
#include "core/rand.hpp"

#include "event/core_events.hpp"

namespace other {

  Directory::Directory() : handle(0) {
    Initialize(false);
  }

  Directory::Directory(const Path& path, UUID hash)
      : handle(hash), proj_relative_path(path) {
    Initialize(true);
  }

  Directory::Directory(Directory* parent, const Path& path, UUID hash)
      : handle(hash), parent_dir(parent), proj_relative_path(path) {
    Initialize(false);
  }

  Directory::Directory(const Ref<Directory>& parent, const Path& path, UUID hash)
      : handle(hash), parent_dir(parent), proj_relative_path(path) {
    Initialize(false);
  }

  void Directory::Poll() {
    if (watcher != nullptr) {
      watcher->Poll();
    }

    for (auto& [id, file] : file_handles) {
      file->Poll();
    }

    for (auto& [id, dir] : children) {
      dir->Poll();
    }
  }

  void Directory::Update() {
    CollectChildren(false);
  }

  Directory::operator Path() const {
    return AbsolutePath();
  }

  std::string Directory::Name() const {
    return proj_relative_path.filename().string();
  }

  bool Directory::Exists() const {
    return std::filesystem::exists(proj_relative_path);
  }

  bool Directory::Contains(const Path& path) const {
    if (!Exists()) {
      return false;
    }

    /// TODO: check if path is a directory or file
    // if (std::filesystem::is_directory(path)) {
    //   return ContainsDirectory(path);
    // } else {
    //   return ContainsFile(path);
    // }

    Path abs_path = std::filesystem::absolute(path);
    if (!std::filesystem::exists(abs_path)) {
      return false;
    }

    // clang-format off
    return (GetFilePaths() | 
      std::views::filter([&abs_path](const auto& f) { return std::filesystem::absolute(f) == abs_path; }) | 
      std::ranges::to<std::vector<Path>>()).size() > 0;
    // clang-format on
  }

  bool Directory::Contains(UUID handle) const {
    return file_handles.find(handle) != file_handles.end();
  }

  Ref<Directory> Directory::AddFolder(const std::string_view name) {
    if (!Exists()) {
      OE_ERROR("Failed to add folder, directory does not exist : {}", proj_relative_path.string());
      return nullptr;
    }

    Path new_dir = proj_relative_path / name;
    UUID hash = FNV(name);
    if (Contains(hash)) {
      return children[hash];
    }

    if (!std::filesystem::exists(new_dir)) {
      std::filesystem::create_directory(new_dir);
    }

    Ref<Directory> dir = children[hash] = NewRef<Directory>(this, new_dir, hash);
    return dir;
  }

  Ref<FileHandle> Directory::AddFile(const std::string_view path) {
    if (!Exists()) {
      OE_ERROR("Failed to add file, directory does not exist : {}", proj_relative_path.string());
      return nullptr;
    }

    Path new_file = proj_relative_path / path;
    UUID hash = FNV(new_file.string());
    if (Contains(hash)) {
      OE_WARN("File already exists : {}", new_file.string());
      return file_handles[hash];
    }

    if (!std::filesystem::exists(new_file)) {
      {
        std::ofstream file{ new_file };
      }
    }
    OE_ASSERT(std::filesystem::exists(new_file), "Failed to create file : {}", new_file.string());

    Ref<FileHandle> file = file_handles[hash] = NewRef<FileHandle>(new_file);
    return file;
  }

  bool Directory::RemoveFile(UUID handle) {
    auto itr = file_handles.find(handle);
    if (itr != file_handles.end()) {
      file_handles.erase(itr);
      return true;
    }

    for (auto& [id, dir] : children) {
      if (dir->RemoveFile(handle)) {
        return true;
      }
    }

    return false;
  }

  bool Directory::RemoveChildDirectory(UUID handle) {
    auto itr = children.find(handle);
    if (itr != children.end()) {
      children.erase(itr);
      return true;
    }
    return false;
  }

  Ref<FileHandle> Directory::GetFile(const Path& path) {
    if (!Exists()) {
      OE_ERROR("Failed to get file, directory does not exist : {}", proj_relative_path.string());
      return nullptr;
    }

    {
      Path test_path = proj_relative_path / path;
      UUID id = FNV(test_path.filename().string());
      if (Contains(id)) {
        return file_handles[id];
      }
    }

    if (!Contains(path)) {
      /// TODO: decide when we want to create the file or fail (check open mode maybe?)
      OE_ERROR("Failed to get file, file not found : {}", path.string());
      return nullptr;
    }

    std::vector<Path> files =
      GetFilePaths() |
      std::views::filter([&path](const auto& f) { return f.filename() == path.filename(); }) |
      std::ranges::to<std::vector<Path>>();

    if (files.size() > 1) {
      OE_ERROR("Failed to get file, multiple files with the same name : {}", path.string());
      return nullptr;
    }

    OE_ASSERT(!files.empty(), "Failed to get file, file not found : {}", path.string());
    OE_ASSERT(files[0] == path, "Failed to get file, file not found : {}", path.string());

    UUID hash = FNV(path.filename().string());
    OE_DEBUG("Getting file : {} ({})", path.filename().string(), hash);
    if (Contains(hash)) {
      return file_handles[hash];
    }

    OE_DEBUG("File Handle Created : {} ({})", path.string(), hash);

    Path real_path = proj_relative_path / path;
    auto handle = NewRef<FileHandle>(real_path);
    handle->handle = hash;
    file_handles[hash] = handle;
    OE_ASSERT(Contains(handle->handle), "Failed to get file : {}", path.string());

    return Ref<FileHandle>::Clone(file_handles[hash]);
  }

  Ref<FileHandle> Directory::GetFile(UUID handle) {
    if (!Exists()) {
      return nullptr;
    }
    if (!Contains(handle)) {
      return nullptr;
    }

    return file_handles[handle];
  }

  Ref<FileHandle> Directory::OpenFile(const Path& path, std::ios_base::openmode mode) {
    if (!Exists()) {
      OE_ERROR("Failed to open file, directory does not exist : {}", proj_relative_path.string());
      return nullptr;
    }

    Ref<FileHandle> file = GetFile(path);
    if (file != nullptr) {
      file->Open(mode);
    }
    return file;
  }

  Ref<FileHandle> Directory::OpenFile(UUID handle, std::ios_base::openmode mode) {
    if (!Exists()) {
      return nullptr;
    }

    Ref<FileHandle> file = GetFile(handle);
    if (file != nullptr) {
      file->Open(mode);
    }
    return file;
  }

  Ref<FileHandle> Directory::GetFileHandleByName(const std::string_view name) {
    if (!Exists()) {
      OE_ERROR("Failed to get file handle, directory does not exist : {}", proj_relative_path.string());
      return nullptr;
    }

    for (auto& [id, file] : file_handles) {
      OE_DEBUG("CHECKING FILE : {} == {}", file->FileName(), name);
      if (file->FileName() == name) {
        return file;
      }
    }

    for (auto& [id, dir] : children) {
      auto file = dir->GetFileHandleByName(name);
      if (file != nullptr) {
        return file;
      }
    }

    return nullptr;
  }

  std::vector<Path> Directory::GetFilePaths(Opt<std::string> ext) const {
    std::vector<Path> files;
    for (auto& entry : std::filesystem::directory_iterator(AbsolutePath())) {
      if (!entry.is_regular_file()) {
        continue;
      }

      Path p = entry.path();
      OE_ASSERT(!p.empty(), "Failed to get file path");

      if (ext.has_value()) {
        if (p.extension() == ext.value()) {
          files.push_back(p.filename());
        }
      } else {
        files.push_back(p.filename());
      }
    }

    return files;
  }

  std::vector<Ref<FileHandle>> Directory::GetFiles(Opt<std::string> ext) const {
    std::vector<Ref<FileHandle>> files;
    for (auto& [id, file] : file_handles) {
      if (ext.has_value()) {
        if (file->Extension() == ext.value()) {
          files.push_back(file);
        }
      } else {
        files.push_back(file);
      }
    }

    for (auto& [id, dir] : children) {
      auto child_files = dir->GetFiles(ext);
      if (child_files.empty()) {
        continue;
      }

      files.insert(files.end(), child_files.begin(), child_files.end());
    }

    return files;
  }

  const Path Directory::AbsolutePath() const {
    if (proj_relative_path.empty()) {
      return Path();
    }
    return std::filesystem::absolute(proj_relative_path);
  }

  const Path Directory::ProjectRelativePath() const {
    if (proj_relative_path.empty()) {
      return Path();
    }
    return proj_relative_path;
  }

  void Directory::Initialize(bool create_dir_handles) {
    if (proj_relative_path.empty()) {
      return;
    }

    auto name = proj_relative_path.stem().filename();

    if (!std::filesystem::exists(proj_relative_path)) {
      OE_WARN("Directory does not exist : {}", proj_relative_path);
      proj_relative_path = Path();
      return;
    }

    watcher = NewRef<DirectoryWatcher>(handle, proj_relative_path);
    CollectChildren(create_dir_handles);
  }

  void Directory::CollectChildren(bool create_dir_handles) {
    if (!Exists()) {
      return;
    }

    OE_TRACE("Collecting Children : {}", proj_relative_path);
    for (auto& entry : std::filesystem::directory_iterator(proj_relative_path)) {
      Path p = entry.path();

      if (entry.is_directory() && create_dir_handles) {
        uint64_t hash = FNV(p.stem().string());
        if (auto itr = children.find(hash); itr != children.end()) {
          continue;
        }
        children[hash] = NewRef<Directory>(this, entry.path(), hash);
      } else if (entry.is_regular_file()) {
        uint64_t hash = FNV(p.filename().string());
        if (auto itr = file_handles.find(hash); itr != file_handles.end()) {
          continue;
        }
        file_handles[hash] = NewRef<FileHandle>(entry.path());
      }
    }
  }

}  // namespace other
