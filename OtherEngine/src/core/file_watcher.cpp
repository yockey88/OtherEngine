/**
 * \file core/file_watcher.cpp
 **/
#include "core/file_watcher.hpp"

#include "core/platform.hpp"

#ifdef OE_WINDOWS
  #include "windows/windows_file_watcher.hpp"
#endif

namespace other {

  FileWatcher::FileWatcher(const std::string_view file_path) {
    
  }

  Scope<FileWatcher> GetFileWatcher(const std::string_view file_path) {
#ifdef OE_WINDOWS
    return NewScope<WindowsFileWatcher>(file_path);
#else
    return nullptr;
#endif
  } 

} // namespace other
