/**
 * \file scripting/language_module.hpp
 * \note language module is a plugin to add custom/per-language util code to the engine 
 **/
#ifndef OTHER_ENGINE_LANGUAGE_MODULE_HPP
#define OTHER_ENGINE_LANGUAGE_MODULE_HPP

#include <map>
#include <type_traits>

#include <core/stable_vector.hpp>

#include "core/uuid.hpp"
#include "core/ref_counted.hpp"
#include "core/ref.hpp"

#include "scripting/script_module.hpp"

namespace other {

  class LanguageModule;

  template <typename T>
  concept lang_module_t = requires {
    std::is_base_of_v<LanguageModule , T>;
  };

  class LanguageModule : public RefCounted {
    public: 
      LanguageModule(LanguageModuleType type) 
          : lang_type(type) {}
      virtual ~LanguageModule() {}

      LanguageModuleType GetLanguageType() const;

      bool HasScript(const std::string_view name);
      bool HasScript(UUID id) const;
      
      const ScriptMetadata* GetScriptMetadata(const std::string& name) const;
      const ScriptMetadata* GetScriptMetadata(const UUID& id) const;

      virtual bool Initialize() = 0;
      virtual void Shutdown() = 0;
      virtual void Reload() = 0;
      virtual Ref<ScriptModule> GetScriptModule(const std::string& name) = 0;
      virtual Ref<ScriptModule> GetScriptModule(const UUID& id) = 0;
      virtual Ref<ScriptModule> LoadScriptModule(const ScriptMetadata& module_info) = 0;
      virtual void UnloadScript(const std::string& name) = 0;

      virtual std::string_view GetModuleName() const = 0;
      virtual std::string_view GetModuleVersion() const = 0;

      std::map<UUID , Ref<ScriptModule>>& GetModules();

    protected:
      LanguageModuleType lang_type;

      std::map<UUID , Ref<ScriptModule>> loaded_modules;
      std::map<UUID , ScriptMetadata> loaded_modules_data;

      bool load_success = false;
  };

} // namespace other

#endif // !OTHER_ENGINE_LANGUAGE_MODULE_HPP
