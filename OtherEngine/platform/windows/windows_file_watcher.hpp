/**
 * \file platform/windows/windows_file_watcher.hpp
 **/
#ifndef OTHER_ENGINE_WINDOWS_FILE_WATCHER_HPP
#define OTHER_ENGINE_WINDOWS_FILE_WATCHER_HPP

#include "core/file_watcher.hpp"

namespace other {

  class WindowsFileWatcher : public FileWatcher {
    public:
      WindowsFileWatcher(const std::string_view file_path)
        : FileWatcher(file_path) {}
  };

} // namespace other

#endif // !OTHER_ENGINE_WINDOWS_FILE_WATCHER_HPP
