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
    last_write = std::filesystem::last_write_time(Path{ file_path });
    exists = true;
    handle = hash;
  }

  bool FileWatcher::Poll() {
    if (exists && !std::filesystem::exists(file_path)) {
      exists = false;
      EventQueue::PushEvent<DeleteFileEvent>({ handle.Get() });
      return false;
    }

    if (!exists) {
      return false;
    }

    bool changed = false;
    auto new_write_time = std::filesystem::last_write_time(file_path);
    if (last_write != new_write_time) {
      changed = true;
      EventQueue::PushEvent<ModifyFileEvent>({ handle.Get() });
    }

    last_write = new_write_time;
    return changed;
  }

}  // namespace other
