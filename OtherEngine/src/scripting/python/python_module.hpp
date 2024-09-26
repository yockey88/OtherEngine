
/**
 * \file scripting/python/python_module.cpp
 **/
#ifndef OTHER_ENGINE_PYTHON_MODULE_HPP
#define OTHER_ENGINE_PYTHON_MODULE_HPP

#include "scripting/language_module.hpp"
#include "scripting/script_defines.hpp"

namespace other {

  class PythonModule : public LanguageModule {
    public:
      PythonModule()
        : LanguageModule(LanguageModuleType::PYTHON_MODULE) {}
      virtual ~PythonModule() override {}

      virtual bool Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual ScriptModule* GetScriptModule(const std::string& name) override;
      virtual ScriptModule* GetScriptModule(const UUID& id) override;
      virtual ScriptModule* LoadScriptModule(const ScriptMetadata& module_info) override;

      virtual void UnloadScript(const std::string& name) override;

      virtual std::string_view GetModuleName() const override;
      virtual std::string_view GetModuleVersion() const override;
  };

} // namespace other

#endif // !OTHER_ENGINE_PYTHON_MODULE_HPP
