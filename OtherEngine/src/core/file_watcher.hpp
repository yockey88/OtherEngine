/**
 * \file core/file_watcher.hpp
 **/
#ifndef OTHER_ENGINE_FILE_WATCHER_HPP
#define OTHER_ENGINE_FILE_WATCHER_HPP

#include <filesystem>

#include "core/defines.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

namespace other {

  class FileWatcher : public RefCounted {
   public:
    FileWatcher(UUID handle, const Path& path);
    virtual ~FileWatcher() = default;

    bool Poll();

   protected:
    bool exists = false;
    UUID handle = 0;
    Path file_path;
    std::filesystem::file_time_type last_write;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_FILE_WATCHER_HPP
