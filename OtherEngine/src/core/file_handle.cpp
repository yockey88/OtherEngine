/**
 * \file core/file_handle.cpp
 **/
#include "core/file_handle.hpp"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <filesystem>

#include "core/defines.hpp"
#include "core/filesystem.hpp"

#include "asset/asset_database.hpp"
#include "asset/asset_manager.hpp"
#include "event/core_events.hpp"
#include "event/event_queue.hpp"

namespace other {

  FileHandle::FileHandle(const Path& path, Opt<std::ios_base::openmode> m) {
    project_relative_path = path;
    handle = Filesystem::GetPathHandle(path);

    if (!Exists() && m.has_value() && (*m & std::ios_base::out)) {
      std::ofstream file(AbsolutePath());
    }
    OE_ASSERT(Exists(), "Can not create file handle for non-existent file : {}", project_relative_path.string());

    asset_type = AssetManager::AssetTypeFromExtension(project_relative_path.extension().string());
    OE_TRACE("FileHandle : {} ({} => {}) [{}]", project_relative_path.string(), project_relative_path.extension().string(), asset_type, handle);

    AssetDatabase::RegisterAsset(this);

    try {
      watcher = NewRef<FileWatcher>(handle, AbsolutePath());
    } catch (std::exception& e) {
      OE_ERROR("Failed to create file watcher : {}", e.what());
      watcher = nullptr;
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
      OE_TRACE("Opened file : {}", project_relative_path.string());
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

  bool FileHandle::Remove() {
    if (!Exists()) {
      return true;
    }

    if (IsOpen()) {
      Close();
    }

    AssetKey key = { .file_handle = handle, .type = asset_type };
    AppState::Assets()->Remove(key);

    try {
      std::filesystem::remove(AbsolutePath());
      return !Exists();
    } catch (std::filesystem::filesystem_error& e) {
      OE_ERROR("Failed to remove file : {} [{}]", project_relative_path.string(), e.what());
      return false;
    } catch (...) {
      OE_ERROR("Failed to remove file : {} [{}]", project_relative_path.string(), std::strerror(errno));
      return false;
    }
  }

  FileHandle::operator Path() const {
    return AbsolutePath();
  }

  AssetType FileHandle::GetAssetType() const {
    return asset_type;
  }

  void FileHandle::Poll() {
    PROFILE_SECTION("FileHandle--Poll");
    if (watcher == nullptr) {
      return;
    }

    if (!watcher->Poll()) {
      return;
    }

    EventQueue::PushEvent<FileModified>({ handle.Get() });
  }

  std::string FileHandle::Extension() const {
    if (handle.Get() == 0) {
      return "";
    }

    return project_relative_path.extension().string();
  }

  std::string FileHandle::FileName() const {
    if (handle.Get() == 0) {
      return "";
    }

    return project_relative_path.stem().string();
  }

  Path FileHandle::AbsolutePath() const {
    return std::filesystem::absolute(project_relative_path);
  }

  Path FileHandle::ProjectRelativePath() const {
    if (handle.Get() == 0) {
      return Path();
    }

    return project_relative_path;
  }

  std::istream& FileHandle::GetReadStream() {
    return file;
  }

  std::ostream& FileHandle::GetWriteStream() {
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
    if (!Exists()) {
      OE_ERROR("Failed to read file : {}", project_relative_path.string());
      return "";
    }

    if (!IsOpen()) {
      OE_ERROR("Failed to read file : {}", project_relative_path.string());
      return "";
    }

    OE_DEBUG("Reading file : {}", project_relative_path.string());
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
  }

  std::vector<char> FileHandle::ReadChars() {
    if (!Exists()) {
      OE_ERROR("Failed to read file : {}", project_relative_path.string());
      return {};
    }

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

  std::vector<uint8_t> FileHandle::ReadBytes() {
    if (!Exists()) {
      OE_ERROR("Failed to read file : {}", project_relative_path.string());
      return {};
    }

    Close();
    Open(std::ios_base::in | std::ios_base::binary);

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

    std::vector<uint8_t> buffer{};
    buffer.resize(size);

    file.read(reinterpret_cast<char*>(buffer.data()), size);
  }

}  // namespace other