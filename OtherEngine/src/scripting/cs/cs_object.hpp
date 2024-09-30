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

      virtual void InitializeScriptMethods() override;
      virtual void InitializeScriptFields() override;
      virtual void UpdateNativeFields() override;
       
      virtual Opt<Value> GetField(const std::string& name) override;
      virtual void SetField(const std::string& name , const Value& value) override;

      // virtual void OnBehaviorLoad() override;
      // virtual void Initialize() override;
      // virtual void Shutdown() override;
      // virtual void OnBehaviorUnload() override;

      // virtual void Start() override;
      // virtual void Stop() override; 

      // virtual void EarlyUpdate(float dt) override;
      // virtual void Update(float dt) override;
      // virtual void LateUpdate(float dt) override;

      // virtual void Render() override;
      // virtual void RenderUI() override;

    private:
      Type& type;
      HostedObject hosted_object;

      virtual void OnSetEntityId() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
