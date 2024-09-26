/**
 * \file scripting/script_module.hpp
 * \note a script module is a single translation unit of the scripting language 
 **/
#ifndef OTHER_ENGINE_SCRIPT_MODULE_HPP
#define OTHER_ENGINE_SCRIPT_MODULE_HPP

#include <string>
#include <string_view>
#include <vector>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/script_object.hpp"

namespace other {

  enum class ScriptType {
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
    ScriptType type = ScriptType::SCENE_SCRIPT;
  };

  struct ScriptMetadata {
    std::string name;
    std::vector<std::string> paths;
    ScriptType type = ScriptType::SCENE_SCRIPT;
  };
  
  class ScriptModule : public RefCounted {
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
      virtual bool HasScript(UUID id) const = 0;
      virtual bool HasScript(const std::string_view name , const std::string_view nspace = "") const = 0;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") = 0;

      virtual std::vector<ScriptObjectTag> GetObjectTags() = 0;

      bool IsValid() const { return valid; }
    
    protected:
      LanguageModuleType language;
      bool valid = false;

      std::string module_name;
      
      std::map<UUID , Ref<ScriptObject>> objects;
  };
  
} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_MODULE_HPP
