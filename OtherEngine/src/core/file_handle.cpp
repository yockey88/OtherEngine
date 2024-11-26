/**
 * \file core/file_handle.cpp
 **/
#include "core/file_handle.hpp"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <filesystem>

#include "core/filesystem.hpp"

#include "asset/asset_database.hpp"
#include "asset/asset_manager.hpp"
#include "event/core_events.hpp"
#include "event/event_queue.hpp"

namespace other {

  FileHandle::FileHandle(UUID hash, const Path& path, Opt<std::ios_base::openmode> m) {
    project_relative_path = path;
    handle = hash;

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

    if (!Filesystem::FileExists(AbsolutePath())) {
      if (m.has_value() && (*m & std::ios_base::out)) {
        std::ofstream file(AbsolutePath());
        file.close();
      } else {
        OE_ERROR("Failed to open file : {}", project_relative_path.string());
        return;
      }
    }
    OE_ASSERT(Filesystem::FileExists(AbsolutePath()), "Failed to create file : {}", project_relative_path.string());

    try {
      watcher = NewRef<FileWatcher>(handle, AbsolutePath());
    } catch (std::exception& e) {
      OE_ERROR("Failed to create file watcher : {}", e.what());
    }

    if (!m.has_value()) {
      return;
    }

    Open(*m);
  }

  FileHandle::~FileHandle() {
    Close();
    handle = 0;
    watcher = nullptr;
    project_relative_path = Path();
  }

  void FileHandle::Open(std::ios_base::openmode new_mode) {
    if (IsOpen()) {
      Close();
    }

    file.open(AbsolutePath(), mode);
    mode = new_mode;

    if (!IsOpen()) {
      OE_ERROR("Failed to open file : {} [{}]", project_relative_path.string(), std::strerror(errno));
    } else {
      OE_DEBUG("Opened file : {}", project_relative_path.string());
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

  FileHandle::operator Path() const {
    return AbsolutePath();
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
    return std::filesystem::absolute(project_relative_path);
  }

  const Path FileHandle::ProjectRelativePath() const {
    if (handle.Get() == 0) {
      return Path();
    }

    return project_relative_path;
  }

  std::fstream& FileHandle::RawFileStream() {
    return file;
  }

  void FileHandle::WriteJson(const nlohmann::json& data) {
    if (Extension() != ".json") {
      OE_ERROR("File {} is not a json file", project_relative_path.string());
      return;
    }

    {
      std::ofstream write_file(AbsolutePath());
      if (!write_file.is_open()) {
        OE_ERROR("Failed to write to file : {} [{}]", project_relative_path.string(), std::strerror(errno));
        return;
      }

      write_file << data.dump(2);
      if (write_file.fail()) {
        OE_ERROR("Failed to write to file : {} [{}]", project_relative_path.string(), std::strerror(errno));
      }
    }
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