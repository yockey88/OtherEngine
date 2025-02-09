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
    if (!std::filesystem::exists(file_path)) {
      exists = false;
    } else {
      exists = true;
      last_write = std::filesystem::last_write_time(Path{ file_path });
    }
    handle = hash;
  }

  bool FileWatcher::Poll() {
    if (!exists) {
      return false;
    }

    if (exists && !std::filesystem::exists(file_path)) {
      exists = false;
      EventQueue::PushEvent<DeleteFileEvent>({ handle.Get() });
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
