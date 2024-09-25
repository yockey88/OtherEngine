/**
 * \file scripting/cs/cs_module.hpp
*/
#ifndef OTHER_ENGINE_CS_MODULE_HPP
#define OTHER_ENGINE_CS_MODULE_HPP

#include <string_view>

#include "scripting/language_module.hpp"

namespace other {

  class Engine;

  class CsModule : public LanguageModule {
    public:
      CsModule() 
        : LanguageModule(LanguageModuleType::CS_MODULE) {}
      virtual ~CsModule() override {}

      virtual bool Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual ScriptModule* GetScript(const std::string& name) override;
      virtual ScriptModule* GetScript(const UUID& id) override;
      virtual ScriptModule* LoadScript(const ScriptMetadata& module_info) override;
      virtual void UnloadScript(const std::string& name) override; 

      virtual std::string_view GetModuleName() const override { return kModuleName; }
      virtual std::string_view GetModuleVersion() const override { return kModuleVersion; }

    private:
      constexpr static std::string_view kModuleName = "C#";
      constexpr static std::string_view kModuleVersion = "0.0.1";

      bool load_success = false;
  };

} // namespace other

#endif // !OTHER_ENGINE_CS_MODULE_HPP
