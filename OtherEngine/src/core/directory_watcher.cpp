/**
 * \file core/directory_watcher.cpp
 **/
#include "core/directory_watcher.hpp"
#include <filesystem>
#include <string_view>

namespace other {
      
  DirectoryWatcher::DirectoryWatcher(const std::string_view dir_path , const std::string_view extension)
      : path(dir_path) , extension(extension) {
    if (!std::filesystem::exists(dir_path)) {
      dir_open = false;
    }
    CollectFiles();
  }

  bool DirectoryWatcher::DirectoryChanged() {
    if (!dir_open) {
      return false;
    }

    /// save data
    size_t pre_check = num_files_last_check;
    std::set<Path> old_files = files;

    /// collect files
    CollectFiles();

    /// early exit if different amount
    if (pre_check != num_files_last_check) {
      return true;
    }

    /// check to see if deleted any files
    for (const auto& ofile : old_files) {
      if (files.find(ofile) == files.end()) {
        return true;
      }
    }
    
    /// check to see if added any files
    for (const auto& file : files) {
      if (old_files.find(file) == old_files.end()) {
        return true;
      }
    }

    return false;
  }
      
  const std::set<Path>& DirectoryWatcher::ViewFiles() const {
    return files;
  }
      
  void DirectoryWatcher::CollectFiles() {
    if (!dir_open) {
      return;
    }

    for (auto entry : std::filesystem::directory_iterator(path)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      
      if (!extension.empty() && entry.path().filename().extension().string() != extension) {
        continue;
      }

      files.insert(entry.path());
      num_files_last_check = files.size();
    }
  }

} // namespace other
