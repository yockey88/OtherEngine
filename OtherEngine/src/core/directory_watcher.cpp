/**
 * \file core/directory_watcher.cpp
 **/
#include "core/directory_watcher.hpp"

#include <filesystem>

#include "core/defines.hpp"

#include "event/core_events.hpp"
#include "event/event_queue.hpp"

#include "rendering/renderer.hpp"

namespace other {

  DirectoryWatcher::DirectoryWatcher(UUID dir_handle, const Path& dir_path)
      : path(dir_path), handle(dir_handle) {
    if (!std::filesystem::exists(dir_path)) {
      dir_open = false;
    } else {
      dir_open = true;
      num_files_last_check = CheckNumFiles();
    }
  }

  void DirectoryWatcher::Poll() {
    /// no update events if window is not focused
    if (!dir_open && !Renderer::IsWindowFocused()) {
      return;
    }

    size_t new_num = CheckNumFiles();
    if (new_num == num_files_last_check) {
      return;
    }

    bool add_file = new_num > num_files_last_check;
    num_files_last_check = new_num;

    if (add_file) {
      EventQueue::PushEvent<CreateFileEvent>({ handle.Get() });
    }
  }

  using fp = bool (*)(const Path&);
  size_t DirectoryWatcher::CheckNumFiles() {
    if (!dir_open) {
      return 0;
    }

    return (size_t)std::count_if(
      std::filesystem::recursive_directory_iterator(path),
      std::filesystem::recursive_directory_iterator{},
      (fp)std::filesystem::is_regular_file
    );
  }

}  // namespace other
