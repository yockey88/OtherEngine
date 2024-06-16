/**
 * \file scripting/script_module.hpp
 * \note a script module is a single translation unit of the scripting language 
 **/
#ifndef OTHER_ENGINE_SCRIPT_MODULE_HPP
#define OTHER_ENGINE_SCRIPT_MODULE_HPP

#include <string>
#include <vector>

#include "core/file_watcher.hpp"
#include "scripting/script_object.hpp"

namespace other {
  
  struct ScriptModuleInfo {
    std::string name;
    std::vector<std::string> paths;
  };
  
  class ScriptModule {
    public:
      ScriptModule() {}
      virtual ~ScriptModule() {}

      bool HasChanged() const;

      void Update();

      const std::string& ModuleName() const;
    
      virtual void Initialize() = 0;
      virtual void Shutdown() = 0;
      virtual void Reload() = 0;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") = 0;

      bool IsValid() const { return valid; }
    
    protected:
      bool valid = false;
      bool changed_on_disk = false;

      std::string module_name;
      Scope<FileWatcher> file_watcher;

      void SetPath(const std::string& path);
  };
  
} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_MODULE_HPP
