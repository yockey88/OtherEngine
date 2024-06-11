/**
 * \file scripting/script_module.cpp
 **/
#include "scripting/script_module.hpp"


namespace other {

  bool ScriptModule::HasChanged() const {
    return changed_on_disk;
  }

  void ScriptModule::Update() {
    for (auto& fw : file_watchers) {
      if (fw == nullptr) {
        continue;
      }

      changed_on_disk = fw->ChangedSinceLastCheck();
      if (changed_on_disk) {
        // don't need to know any more other than that one file changed
        break;
      }
    }
  }
      
  void ScriptModule::SetPaths(const std::vector<std::string>& paths) {
    for (auto& p : paths) {
      file_watchers.emplace_back(NewScope<FileWatcher>(p));
    }
  }

} // namespace other
