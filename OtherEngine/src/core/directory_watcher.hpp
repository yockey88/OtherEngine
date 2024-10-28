/**
 * \file core/directory_watcher.hpp
 **/
#ifndef OTHER_ENGINE_DIRECTORY_WATCHER_HPP
#define OTHER_ENGINE_DIRECTORY_WATCHER_HPP

#include <set>
#include <string_view>

#include "core/defines.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

namespace other {

  class DirectoryWatcher : public RefCounted {
   public:
    DirectoryWatcher(UUID dir_handle, const Path& dir_path);
    virtual ~DirectoryWatcher() {}

    void Poll();

   protected:
    bool dir_open = true;
    Path path;
    UUID handle;

    size_t num_files_last_check = 0;

    size_t CheckNumFiles();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_DIRECTORY_WATCHER_HPP
