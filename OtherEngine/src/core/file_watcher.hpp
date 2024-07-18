/**
 * \file core/file_watcher.hpp
 **/
#ifndef OTHER_ENGINE_FILE_WATCHER_HPP
#define OTHER_ENGINE_FILE_WATCHER_HPP

#include <filesystem>
#include <string_view>

namespace other {

  class FileWatcher {
    public:
      FileWatcher(const std::string_view file_path);

      bool ChangedSinceLastCheck();

    protected:
      std::string file_path;
      std::filesystem::file_time_type last_write;
  }; 

} // namespace other

#endif // !OTHER_ENGINE_FILE_WATCHER_HPP
