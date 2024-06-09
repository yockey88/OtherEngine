/**
 * \file scripting/script_module.hpp
 * \note a script module is a single translation unit of the scripting language 
 **/
#ifndef OTHER_ENGINE_SCRIPT_MODULE_HPP
#define OTHER_ENGINE_SCRIPT_MODULE_HPP

#include <string>
#include <vector>

#include "plugin/plugin.hpp"
#include "scripting/script_object.hpp"

namespace other {
  
  struct ScriptModuleInfo {
    std::string name;
    std::vector<std::string> paths;
  };
  
  class ScriptModule : public Plugin {
    public:
      ScriptModule(Engine* engine) 
        : Plugin(engine) {}
      virtual ~ScriptModule() override {}
    
      virtual void Initialize() = 0;
      virtual void Shutdown() = 0;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") = 0;

      bool IsValid() const { return valid; }
    
    protected:
      bool valid = false;
  };
  
} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_MODULE_HPP
