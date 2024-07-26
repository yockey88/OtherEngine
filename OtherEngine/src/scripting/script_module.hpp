/**
 * \file scripting/script_module.hpp
 * \note a script module is a single translation unit of the scripting language 
 **/
#ifndef OTHER_ENGINE_SCRIPT_MODULE_HPP
#define OTHER_ENGINE_SCRIPT_MODULE_HPP

#include <string>
#include <string_view>
#include <vector>

#include "scripting/script_defines.hpp"
#include "scripting/script_object.hpp"

namespace other {

  enum class ScriptModuleType {
    SCENE_SCRIPT , 
    EDITOR_SCRIPT ,

    NUM_SCRIPT_TYPES , 
    INVALID_SCRIPT_TYPE = NUM_SCRIPT_TYPES ,
  };

  struct ScriptObjectTag {
    UUID object_id;
    std::string name;
    std::string mod_name;
    std::string nspace;
    std::string path;
    LanguageModuleType lang_type;
    ScriptModuleType type = ScriptModuleType::SCENE_SCRIPT;
  };

  struct ScriptModuleInfo {
    std::string name;
    std::vector<std::string> paths;
    ScriptModuleType type = ScriptModuleType::SCENE_SCRIPT;
  };
  
  class ScriptModule {
    public:
      ScriptModule(LanguageModuleType language , const std::string& module_name) 
          : language(language) , module_name(module_name) {}
      virtual ~ScriptModule() {}

      LanguageModuleType GetLanguage() const {
        return language;
      }
      
      const std::string& ModuleName() const {
        return module_name;
      }
    
      virtual void Initialize(/* bool editor_script = false */) = 0;
      virtual void Shutdown() = 0;
      virtual void Reload() = 0;
      virtual bool HasScript(UUID id) = 0;
      virtual bool HasScript(const std::string_view name , const std::string_view nspace = "") = 0;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") = 0;

      virtual std::vector<ScriptObjectTag> GetObjectTags() = 0;

      bool IsValid() const { return valid; }
    
    protected:
      LanguageModuleType language;
      bool valid = false;

      std::string module_name;
  };
  
} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_MODULE_HPP
