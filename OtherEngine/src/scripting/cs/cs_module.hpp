/**
 * \file scripting/cs/cs_module.hpp
*/
#ifndef OTHER_ENGINE_CS_MODULE_HPP
#define OTHER_ENGINE_CS_MODULE_HPP

#include <string_view>

#include <mono/metadata/appdomain.h>

#include "scripting/language_module.hpp"

namespace other {

  class Engine;

  class CsModule : public LanguageModule {
    public:
      CsModule() 
        : LanguageModule() {}
      virtual ~CsModule() override {}

      virtual bool Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual ScriptModule* GetScriptModule(const std::string& name) override;
      virtual ScriptModule* GetScriptModule(const UUID& id) override;
      virtual ScriptModule* LoadScriptModule(const ScriptModuleInfo& module_info) override;
      virtual void UnloadScriptModule(const std::string& name) override; 

      virtual std::string_view GetModuleName() const override { return kModuleName; }
      virtual std::string_view GetModuleVersion() const override { return kModuleVersion; }

    private:
      constexpr static std::string_view kModuleName = "C#";
      constexpr static std::string_view kModuleVersion = "0.0.1";
      constexpr static std::string_view kMonoPath = "externals";
      constexpr static std::string_view kMonoConfigPath = "externals/mono/etc/mono/config";
      constexpr static std::string_view kRootDomainName = "OTHER-ROOT-DOMAIN";
      constexpr static std::string_view kAppDomainName = "OTHER-APP-DOMAIN";

      std::filesystem::path mono_path;
      std::filesystem::path mono_config_path;

      MonoDomain* root_domain = nullptr;
      MonoDomain* app_domain = nullptr; 

      std::map<UUID , ScriptModuleInfo> loaded_modules_data;

      bool load_success = false;
  };

} // namespace other

#endif // !OTHER_ENGINE_CS_MODULE_HPP
