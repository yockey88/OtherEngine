/**
 * \file core/directory.cpp
 **/
#include "core/directory.hpp"

#include <filesystem>

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

  bool Directory::Exists() const {
    return std::filesystem::exists(proj_relative_path);
  }

  bool Directory::Contains(const Path& path) const {
    return !(GetFiles() | std::views::filter([&path](const auto& f) { return f.filename() == path.filename(); })).empty();
  }

  bool Directory::Contains(UUID handle) const {
    return file_handles.find(handle) != file_handles.end();
  }

  Ref<FileHandle> Directory::OpenFile(const Path& path, std::ios_base::openmode mode) {
    if (!Exists()) {
      OE_ERROR("Failed to open file, directory does not exist : {}", proj_relative_path.string());
      return nullptr;
    }

    if (!Contains(path)) {
      OE_ERROR("Failed to open file, file not found : {} ({})", path.string(), proj_relative_path / path);
      return nullptr;
    }

    std::vector<Path> files =
      GetFiles() |
      std::views::filter([&path](const auto& f) { return std::filesystem::absolute(f) == std::filesystem::absolute(path); }) |
      std::ranges::to<std::vector<Path>>();
    OE_ASSERT(files.size() == 1, "Failed to open file, multiple files with the same name : {}", path.string());
    OE_ASSERT(files[0] == path, "Failed to open file, file not found : {}", path.string());

    UUID hash = FNV(path.filename().string());
    OE_DEBUG("Opening file : {} ({})", path.filename().string(), hash);
    if (Contains(hash)) {
      auto& file = file_handles[hash];
      if (file->IsOpen()) {
        file->Close();
      }
      file->Open(mode);

      OE_DEBUG("File Opened : {} ({})", path.string(), hash);
      return file_handles[hash];
    }

    OE_DEBUG("File Handle Created : {} ({})", path.string(), hash);
    OE_DEBUG("File Opened : {} ({})", path.string(), hash);

    Path real_path = proj_relative_path / path;
    auto handle = Ref<FileHandle>::Create(hash, real_path, mode);
    handle->handle = hash;
    file_handles[hash] = handle;
    OE_ASSERT(Contains(handle->handle), "Failed to open file : {}", path.string());

    return Ref<FileHandle>::Clone(file_handles[hash]);
  }

  Ref<FileHandle> Directory::OpenFile(UUID handle, std::ios_base::openmode mode) {
    if (!Exists()) {
      return nullptr;
    }
    if (!Contains(handle)) {
      return nullptr;
    }

    return file_handles[handle];
  }

  std::vector<Path> Directory::GetFiles(Opt<std::string> ext) const {
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

    for (auto& entry : std::filesystem::directory_iterator(proj_relative_path)) {
      if (entry.is_directory()) {
        Path p = entry.path();
        children[FNV(p.string())] = NewRef<Directory>(this, entry.path());
      } else if (entry.is_regular_file()) {
        paths.push_back(entry.path());
      }
    }
  }

}  // namespace other
