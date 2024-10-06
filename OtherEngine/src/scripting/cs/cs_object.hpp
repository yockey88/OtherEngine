/**
 * \file scripting/cs/cs_object.hpp
*/
#ifndef OTHER_ENGINE_CS_OBJECT_HPP
#define OTHER_ENGINE_CS_OBJECT_HPP

#include "hosting/type.hpp"
#include "hosting/hosted_object.hpp"

#include "scripting/script_object.hpp"

using dotother::Type;
using dotother::HostedObject;

namespace other {

  class CsObject : public ScriptObjectHandle<CsObject> {
    public:
      CsObject(ScriptModule* module , Type& type , UUID handle);
      virtual ~CsObject() override {}
      
      template <typename R , typename... Args>
      R CallMethod(const std::string_view name , Args&&... args) {
        if constexpr (std::same_as<R , void>) {
          hosted_object.Invoke<void>(name , std::forward<Args>(args)...);
          return;
        }

        return hosted_object.Invoke<R>(name , std::forward<Args>(args)...);
      }

      template <typename T>
      void SetField(const std::string_view name , T&& value) {
        hosted_object.SetField(name , std::forward<T>(value));
      }

      template <typename R>
      R GetField(const std::string_view name) {
        return hosted_object.GetField<R>(name);
      }
      
      template <typename T>
      void SetProperty(const std::string_view name , T&& value) {
        hosted_object.SetProperty(name , std::forward<T>(value));
      }

      template <typename R>
      R GetProperty(const std::string_view name) {
        return hosted_object.GetProperty<R>(name);
      }

      virtual void InitializeScriptMethods() override;
      virtual void InitializeScriptFields() override;
      virtual void UpdateNativeFields() override;

    private:
      Type& type;
      HostedObject hosted_object;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
