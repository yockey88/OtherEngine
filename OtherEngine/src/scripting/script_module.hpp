/**
 * \file scripting/script_module.hpp
 * \note a script module is a single translation unit of the scripting language 
 **/
#ifndef OTHER_ENGINE_SCRIPT_MODULE_HPP
#define OTHER_ENGINE_SCRIPT_MODULE_HPP

#include <string>
#include <vector>

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
    
      virtual void Initialize() = 0;
      virtual void Shutdown() = 0;
      virtual void Reload() = 0;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") = 0;

      bool IsValid() const { return valid; }
    
    protected:
      bool valid = false;
      bool changed_on_disk = false;
  };
  
} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_MODULE_HPP
