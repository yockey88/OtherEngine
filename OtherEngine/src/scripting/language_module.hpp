/**
 * \file scripting/language_module.hpp
 * \note language module is a plugin to add custom/per-language util code to the engine 
 **/
#ifndef OTHER_ENGINE_LANGUAGE_MODULE_HPP
#define OTHER_ENGINE_LANGUAGE_MODULE_HPP

#include <map>

#include "core/engine.hpp"
#include "core/uuid.hpp"
#include "plugin/plugin.hpp"
#include "scripting/script_module.hpp"

namespace other {

  class LanguageModule : public Plugin {
    public: 
      LanguageModule(Engine* engine)
        : Plugin(engine) {}
      virtual ~LanguageModule() override {}

      bool HasScriptModule(const std::string_view name) const;
      bool HasScriptModule(UUID id) const;

      virtual bool Initialize() = 0;
      virtual bool Reinitialize() = 0;
      virtual void Shutdown() = 0;
      virtual ScriptModule* GetScriptModule(const std::string& name) = 0;
      virtual ScriptModule* GetScriptModule(const UUID& id) = 0;
      virtual ScriptModule* LoadScriptModule(const ScriptModuleInfo& module_info) = 0;
      virtual void UnloadScriptModule(const std::string& name) = 0;

      virtual std::string_view GetModuleName() const = 0;
      virtual std::string_view GetModuleVersion() const = 0;

    protected:
      std::map<UUID , ScriptModule*> loaded_modules;

      bool load_success = false;
  };

} // namespace other

#endif // !OTHER_ENGINE_LANGUAGE_MODULE_HPP
