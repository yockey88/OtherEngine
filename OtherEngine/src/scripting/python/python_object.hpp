
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
      PythonObject(ScriptModule* module , UUID handle , const std::string& instance_name)
        : ScriptObject(LanguageModuleType::PYTHON_MODULE , module , handle , instance_name) {}
      virtual ~PythonObject() override {}

      template <typename R , typename... Args>
      R CallMethod(const std::string_view method , Args&&... args) {
        if constexpr (std::is_same_v<R , void>) {
          return;
        }
        
        return R{};
      }
      
      virtual void InitializeScriptMethods() override {}
      virtual void InitializeScriptFields() override {}
      virtual void UpdateNativeFields() override {}
      
      virtual Opt<Value> GetField(const std::string& name) override { return std::nullopt; }
      virtual void SetField(const std::string& name , const Value& value) override {}

      // virtual void OnBehaviorLoad() override {}
      // virtual void Initialize() override {}
      // virtual void Shutdown() override {}
      // virtual void OnBehaviorUnload() override {}

      // virtual void Start() override {}
      // virtual void Stop() override {}

      // virtual void EarlyUpdate(float dt) override {}
      // virtual void Update(float dt) override {}
      // virtual void LateUpdate(float dt) override {}

      // virtual void Render() override {}
      // virtual void RenderUI() override {}
  };

} // namespace other

#endif // !OTHER_ENGINE_PYTHON_OBJECT_HPP
