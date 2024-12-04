/**
 * \file core/directory.cpp
 **/
#include "core/directory.hpp"

#include <filesystem>
#include <ranges>

#include "core/logger.hpp"
#include "core/rand.hpp"

namespace other {

  Directory::Directory() {
    handle = Random::GenerateUUID();
  }

  Directory::Directory(const Path& path)
      : proj_relative_path(path) {
    Initialize();
  }

  Directory::Directory(Directory* parent, const Path& path)
      : parent_dir(parent), proj_relative_path(path) {
    Initialize();
  }

  Directory::Directory(const Ref<Directory>& parent, const Path& path)
      : parent_dir(parent), proj_relative_path(path) {
    Initialize();
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
    return !(GetFilePaths() | std::views::filter([&path](const auto& f) { return f.filename() == path.filename(); })).empty();
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
    UUID hash = FNV(new_dir.string());
    if (Contains(hash)) {
      return children[hash];
    }

    if (!std::filesystem::exists(new_dir)) {
      std::filesystem::create_directory(new_dir);
    }

    Ref<Directory> dir = children[hash] = NewRef<Directory>(this, new_dir);
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

    Ref<FileHandle> file = file_handles[hash] = NewRef<FileHandle>(hash, new_file);
    return file;
  }

  Ref<FileHandle> Directory::GetFile(const Path& path) {
    if (!Exists()) {
      OE_ERROR("Failed to get file, directory does not exist : {}", proj_relative_path.string());
      return nullptr;
    }

    if (!Contains(path)) {
      Path file_path = AbsolutePath() / path;
      { std::ofstream file(file_path); }
      if (!Contains(path)) {
        OE_ERROR("Failed to get file, file not found : {}", path.string());
        return nullptr;
      }
    }

    std::vector<Path> files =
      GetFilePaths() |
      std::views::filter([&path](const auto& f) { return std::filesystem::absolute(f) == std::filesystem::absolute(path); }) |
      std::ranges::to<std::vector<Path>>();
    OE_ASSERT(files.size() == 1, "Failed to get file, multiple files with the same name : {}", path.string());
    OE_ASSERT(files[0] == path, "Failed to get file, file not found : {}", path.string());

    UUID hash = FNV(path.filename().string());
    OE_DEBUG("Getting file : {} ({})", path.filename().string(), hash);
    if (Contains(hash)) {
      return file_handles[hash];
    }

    OE_DEBUG("File Handle Created : {} ({})", path.string(), hash);

    Path real_path = proj_relative_path / path;
    auto handle = Ref<FileHandle>::Create(hash, real_path);
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

  void Directory::Initialize() {
    auto name = proj_relative_path.stem().filename();

    handle = FNV(proj_relative_path.string());
    if (!std::filesystem::exists(proj_relative_path)) {
      OE_WARN("Directory does not exist : {}", proj_relative_path);
      proj_relative_path = Path();
      return;
    }

    watcher = NewRef<DirectoryWatcher>(handle, proj_relative_path);

    CollectChildren();
  }

  void Directory::CollectChildren() {
    if (!Exists()) {
      return;
    }

    OE_TRACE("Collecting Children : {}", proj_relative_path);
    for (auto& entry : std::filesystem::directory_iterator(proj_relative_path)) {
      if (entry.is_directory()) {
        Path p = entry.path();
        children[FNV(p.string())] = NewRef<Directory>(this, entry.path());
      } else if (entry.is_regular_file()) {
        Path p = entry.path();
        file_handles[FNV(p.string())] = NewRef<FileHandle>(FNV(p.string()), entry.path());
      }
    }
  }

}  // namespace other
