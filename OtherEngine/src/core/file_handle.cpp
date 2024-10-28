/**
 * \file core/file_handle.cpp
 **/
#include "core/file_handle.hpp"

#include <cstddef>
#include <filesystem>

#include "asset/asset_database.hpp"
#include "asset/asset_manager.hpp"
#include "event/core_events.hpp"
#include "event/event_queue.hpp"

namespace other {

  FileHandle::FileHandle(UUID hash, const Path& path, std::ios_base::openmode mode) {
    project_relative_path = path;
    handle = hash;
    watcher = NewRef<FileWatcher>(handle, AbsolutePath());

    auto ext_t = AssetManager::AssetTypeFromExtension(project_relative_path.extension().string());
    if (ext_t.has_value()) {
      asset_type = ext_t.value();
    }

    if (IsAsset() && !AssetDatabase::Contains(handle)) {
      AssetDatabase::RegisterAsset({
        .handle = handle,
        .type = GetAssetType(),
        .path = project_relative_path,
        .loaded = false,
      });
    }

    Open(mode);

    if (IsOpen()) {
      OE_DEBUG("File Opened : {} ({})", project_relative_path.string(), handle);
    }
  }

  FileHandle::~FileHandle() {
    Close();
    handle = 0;
    watcher = nullptr;
    project_relative_path = Path();
  }

  void FileHandle::Open(std::ios_base::openmode new_mode) {
    if (!Exists()) {
      OE_ERROR("File does not exist : {}", project_relative_path.string());
      return;
    }

    if (IsOpen()) {
      Close();
    }

    file.open(AbsolutePath(), mode);
    mode = new_mode;

    if (!IsOpen()) {
      OE_ERROR("Failed to open file : {}", project_relative_path.string());
      return;
    }
  }

  void FileHandle::Close() {
    file.close();
  }

  bool FileHandle::Exists() const {
    return std::filesystem::exists(AbsolutePath());
  }

  bool FileHandle::IsOpen() const {
    return file.is_open();
  }

  bool FileHandle::IsAsset() const {
    return asset_type.has_value();
  }

  AssetType FileHandle::GetAssetType() const {
    return IsAsset() ? *asset_type : AssetType::GENERIC_FILE;
  }

  void FileHandle::Poll() {
    if (watcher == nullptr) {
      return;
    }

    if (!watcher->Poll()) {
      return;
    }

    if (IsAsset()) {
    } else {
      EventQueue::PushEvent<ModifyFileEvent>({ handle.Get() });
    }
  }

  const std::string FileHandle::Extension() const {
    if (handle.Get() == 0) {
      return "";
    }

    return project_relative_path.extension().string();
  }

  const std::string FileHandle::FileName() const {
    if (handle.Get() == 0) {
      return "";
    }

    return project_relative_path.stem().string();
  }

  const Path FileHandle::AbsolutePath() const {
    if (handle.Get() == 0) {
      return Path();
    }

    return std::filesystem::absolute(project_relative_path);
  }

  const Path FileHandle::ProjectRelativePath() const {
    if (handle.Get() == 0) {
      return Path();
    }

    return project_relative_path;
  }

  std::string FileHandle::ReadString() {
    if (!IsOpen()) {
      OE_ERROR("Failed to read file : {}", project_relative_path.string());
      return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
  }

  std::vector<char> FileHandle::ReadChars() {
    if (!IsOpen()) {
      OE_ERROR("Failed to read file : {}", project_relative_path.string());
      return {};
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size == 0) {
      return {};
    }
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    return buffer;
  }

}  // namespace other