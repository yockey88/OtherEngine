/**
 * \file cs_module.hpp
*/
#ifndef OTHER_ENGINE_CS_MODULE_HPP
#define OTHER_ENGINE_CS_MODULE_HPP

#include <string_view>

#include <mono/metadata/appdomain.h>

#include "core/platform.hpp"
#include "scripting/language_module.hpp"

namespace other {

  class Engine;

  class CsModule : public LanguageModule {
    public:
      CsModule(Engine* engine) 
        : LanguageModule(engine) {}
      virtual ~CsModule() override {}

      virtual void Initialize() override;
      virtual void Reinitialize() override;
      virtual void Shutdown() override;
      virtual ScriptModule* GetScriptModule(const std::string& name) override;
      virtual ScriptModule* GetScriptModule(const UUID& id) override;
      virtual ScriptModule* LoadScriptModule(const ScriptModuleInfo& module_info) override;
      virtual void UnloadScriptModule(const std::string& name) override; 

      virtual std::string_view GetModuleName() const override { return kModuleName; }
      virtual std::string_view GetModuleVersion() const override { return kModuleVersion; }

    private:
      constexpr static std::string_view kModuleName = "C#";
      constexpr static std::string_view kModuleVersion = "0.1";
      constexpr static std::string_view kMonoPath = "OtherEngine-Components/language-modules/cs";
      constexpr static std::string_view kMonoConfigPath = "OtherEngine-Components/language-modules/cs/mono/etc/mono/config";
      constexpr static std::string_view kRootDomainName = "OTHER";
      constexpr static std::string_view kAppDomainName = "OTHER-APP";

      MonoDomain* root_domain = nullptr;
      MonoDomain* app_domain = nullptr; 
  };

} // namespace other

OE_API other::Plugin* CreatePlugin(other::Engine* engine);
OE_API void DestroyPlugin(other::Plugin* module);

#endif // !OTHER_ENGINE_CS_MODULE_HPP
