
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

      template <typename T>
      void SetField(const std::string_view name , T&& value) {
      }

      template <typename R>
      R GetField(const std::string_view name) {
        return R{};
      }

      template <typename T>
      void SetProperty(const std::string_view name , T&& value) {
      }

      template <typename R>
      R GetProperty(const std::string_view name) {
        return R{};
      }

      virtual void InitializeScriptMethods() override {}
      virtual void InitializeScriptFields() override {}
      virtual void UpdateNativeFields() override {}
  };

} // namespace other

#endif // !OTHER_ENGINE_PYTHON_OBJECT_HPP
