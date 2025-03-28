/**
 * \file core/file_watcher.cpp
 **/
#include "core/file_watcher.hpp"

#include <filesystem>

#include "event/core_events.hpp"
#include "event/event_queue.hpp"

#include "rendering/renderer.hpp"

namespace other {

  FileWatcher::FileWatcher(UUID hash, const Path& path) {
    file_path = path;
    exists = std::filesystem::exists(file_path);
    if (exists) {
      last_write = std::filesystem::last_write_time(file_path);
    }

    handle = hash;
  }

  bool FileWatcher::Poll() {
    if (!exists) {
      return false;
    }

    static constexpr std::array kInvisibleExtensions = {
      FNV(".tmp"),
      FNV(".swp"),
      FNV(".swo"),
      FNV(".swn"),
      FNV(".swx"),
      FNV(".swn"),
      FNV(".log"),
      FNV(".ini"),
      FNV(".db"),
    };
    if (std::ranges::find(kInvisibleExtensions, FNV(file_path.extension().string())) != kInvisibleExtensions.end()) {
      return false;
    }

    if (exists && !std::filesystem::exists(file_path)) {
      exists = false;
      EventQueue::PushEvent<FileDeleted>({ handle.Get() });
      return false;
    }

    bool changed = false;
    auto new_write_time = std::filesystem::last_write_time(file_path);
    if (last_write != new_write_time) {
      changed = true;
    }

    last_write = new_write_time;
    return changed;
  }

}  // namespace other
