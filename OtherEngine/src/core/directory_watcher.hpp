/**
 * \file core/directory_watcher.hpp
 **/
#ifndef OTHER_ENGINE_DIRECTORY_WATCHER_HPP
#define OTHER_ENGINE_DIRECTORY_WATCHER_HPP

#include <set>
#include <string_view>

#include "core/defines.hpp"

namespace other {

  class DirectoryWatcher {
    public:
      DirectoryWatcher(const std::string_view dir_path , const std::string_view extension = "");
      virtual ~DirectoryWatcher() {}

      bool DirectoryChanged();

      const std::set<Path>& ViewFiles() const;

    protected:
      bool dir_open = true;
      std::string path;
      std::string extension;

      size_t num_files_last_check = 0;
      std::set<Path> files{};

      void CollectFiles();
  };

} // namespace other

#endif // !OTHER_ENGINE_DIRECTORY_WATCHER_HPP
