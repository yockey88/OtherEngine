
/**
 * \file scripting/python/python_script.cpp
 **/
#ifndef OTHER_ENGINE_PYTHON_SCRIPT_HPP
#define OTHER_ENGINE_PYTHON_SCRIPT_HPP

#if !OE_MODULE
  #include <pybind11/pybind11.h>
#endif

#include "scripting/script_defines.hpp"
#include "scripting/script_module.hpp"

namespace other {

  class PythonScript : public ScriptModule {
    public:
      PythonScript(const std::string& path , const std::string& module_name)
        : ScriptModule(LanguageModuleType::PYTHON_MODULE , module_name) {}
      virtual ~PythonScript() override {} 
      
      virtual void Initialize(/* bool editor_script = false */) override {}
      virtual void Shutdown() override {}
      virtual void Reload() override {}
      virtual bool HasScript(UUID id) const override { return false; }
      virtual bool HasScript(const std::string_view name , const std::string_view nspace = "") const override { return false; }
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") override { return nullptr; }

      virtual std::vector<ScriptObjectTag> GetObjectTags() override { return {}; }

    private:
  };

} // namespace other

#endif // !OTHER_ENGINE_PYTHON_SCRIPT_HPP
