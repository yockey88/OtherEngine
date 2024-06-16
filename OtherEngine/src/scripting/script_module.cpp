/**
 * \file scripting/script_module.cpp
 **/
#include "scripting/script_module.hpp"

#include "event/event_queue.hpp"
#include "event/app_events.hpp"

namespace other {

  bool ScriptModule::HasChanged() const {
    return changed_on_disk;
  }
      
  const std::string& ScriptModule::ModuleName() const {
    return module_name;
  }

  void ScriptModule::Update() {
    if (file_watcher == nullptr) {
      return;
    }

    changed_on_disk = file_watcher->ChangedSinceLastCheck();
    if (changed_on_disk) {
      EventQueue::PushEvent<ScriptReloadEvent>();
      return;
    }
  }
      
  void ScriptModule::SetPath(const std::string& path) {
    module_name = Path{ path }.filename().string();
    module_name = module_name.substr(0 , module_name.find_last_of('.'));
    file_watcher =  NewScope<FileWatcher>(path);
  }

} // namespace other
