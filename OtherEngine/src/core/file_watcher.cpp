/**
 * \file core/file_watcher.cpp
 **/
#include "core/file_watcher.hpp"

#include "core/platform.hpp"
#include <filesystem>

namespace other {

  FileWatcher::FileWatcher(const std::string_view file_path) {
    this->file_path = file_path;
    last_write = std::filesystem::last_write_time(Path{ file_path });
  }

  bool FileWatcher::ChangedSinceLastCheck() {
    bool changed = false;

    auto new_write_time = std::filesystem::last_write_time(file_path);
    if (last_write != new_write_time) {
      changed = true;
    } 

    last_write = new_write_time;
    return changed;
  }

} // namespace other
