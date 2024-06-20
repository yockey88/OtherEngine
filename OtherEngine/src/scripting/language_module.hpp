/**
 * \file scripting/language_module.hpp
 * \note language module is a plugin to add custom/per-language util code to the engine 
 **/
#ifndef OTHER_ENGINE_LANGUAGE_MODULE_HPP
#define OTHER_ENGINE_LANGUAGE_MODULE_HPP

#include <map>
#include <type_traits>

#include "core/uuid.hpp"
#include "core/ref_counted.hpp"
#include "scripting/script_module.hpp"

namespace other {

  class LanguageModule;

  template <typename T>
  concept LangModule = requires {
    std::is_base_of_v<LanguageModule , T>;
  };

  class LanguageModule : public RefCounted {
    public: 
      LanguageModule(LanguageModuleType type) 
          : lang_type(type) {}
      virtual ~LanguageModule() {}

      LanguageModuleType GetLanguageType() const;

      bool HasScriptModule(const std::string_view name) const;
      bool HasScriptModule(UUID id) const;

      void Update();

      virtual bool Initialize() = 0;
      virtual void Shutdown() = 0;
      virtual void Reload() = 0;
      virtual ScriptModule* GetScriptModule(const std::string& name) = 0;
      virtual ScriptModule* GetScriptModule(const UUID& id) = 0;
      virtual ScriptModule* LoadScriptModule(const ScriptModuleInfo& module_info) = 0;
      virtual void UnloadScriptModule(const std::string& name) = 0;

      virtual std::string_view GetModuleName() const = 0;
      virtual std::string_view GetModuleVersion() const = 0;

    protected:
      LanguageModuleType lang_type;

      std::map<UUID , ScriptModule*> loaded_modules;
      std::map<UUID , ScriptModuleInfo> loaded_modules_data;

      bool load_success = false;
  };

} // namespace other

#endif // !OTHER_ENGINE_LANGUAGE_MODULE_HPP
