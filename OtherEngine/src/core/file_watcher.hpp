/**
 * \file core/file_watcher.hpp
 **/
#ifndef OTHER_ENGINE_FILE_WATCHER_HPP
#define OTHER_ENGINE_FILE_WATCHER_HPP

#include <string_view>
#include <chrono>

#include "core/defines.hpp"

namespace other {

  class FileWatcher {
    public:
      FileWatcher(const std::string_view file_path);

      bool ChangedSinceLastCheck();

    private:
      std::string file_path;


  }; 

  Scope<FileWatcher> GetFileWatcher(const std::string_view file_path);

} // namespace other

#endif // !OTHER_ENGINE_FILE_WATCHER_HPP
