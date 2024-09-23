
/**
 * \file scripting/python/python_object.cpp
 **/
#ifndef OTHER_ENGINE_PYTHON_OBJECT_HPP
#define OTHER_ENGINE_PYTHON_OBJECT_HPP

#include "scripting/script_defines.hpp"
#include "scripting/script_object.hpp"

namespace other {

  class PythonObject : public ScriptObject {
    public:
      PythonObject()
        : ScriptObject(LanguageModuleType::PYTHON_MODULE , "[ Empty Python Object ]" , "Python") {}
      PythonObject(const std::string& mod_name, const std::string& name)
        : ScriptObject(LanguageModuleType::PYTHON_MODULE , mod_name , name) {}
      virtual ~PythonObject() override {}
      
      virtual void InitializeScriptMethods() override {}
      virtual void InitializeScriptFields() override {}
      virtual void UpdateNativeFields() override {}
      
      virtual Opt<Value> GetField(const std::string& name) override { return std::nullopt; }
      virtual void SetField(const std::string& name , const Value& value) override {}

      virtual void OnBehaviorLoad() override {}
      virtual void Initialize() override {}
      virtual void Shutdown() override {}
      virtual void OnBehaviorUnload() override {}

      virtual void Start() override {}
      virtual void Stop() override {}

      virtual void EarlyUpdate(float dt) override {}
      virtual void Update(float dt) override {}
      virtual void LateUpdate(float dt) override {}

      virtual void Render() override {}
      virtual void RenderUI() override {}
  };

} // namespace other

#endif // !OTHER_ENGINE_PYTHON_OBJECT_HPP
