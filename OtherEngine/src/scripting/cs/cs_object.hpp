/**
 * \file scripting/cs/cs_object.hpp
*/
#ifndef OTHER_ENGINE_CS_OBJECT_HPP
#define OTHER_ENGINE_CS_OBJECT_HPP

#include "Native/hosting/type.hpp"
#include "Native/hosting/hosted_object.hpp"

#include "core/UUID.hpp"
#include "scripting/script_object.hpp"

using dotother::Type;
using dotother::HostedObject;

namespace other {

  class CsObject : public ScriptObject {
    public:
      CsObject(Type& type);
      virtual ~CsObject() override {}

      virtual void InitializeScriptMethods() override;
      virtual void InitializeScriptFields() override;
      virtual void UpdateNativeFields() override;
       
      virtual Opt<Value> GetField(const std::string& name) override;
      virtual void SetField(const std::string& name , const Value& value) override;

      virtual void OnBehaviorLoad() override;
      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual void OnBehaviorUnload() override;

      virtual void Start() override;
      virtual void Stop() override; 

      virtual void EarlyUpdate(float dt) override;
      virtual void Update(float dt) override;
      virtual void LateUpdate(float dt) override;

      virtual void Render() override;
      virtual void RenderUI() override;

    private:
      Type& type;
      HostedObject hosted_object;

      virtual void OnSetEntityId() override;
      virtual Opt<Value> OnCallMethod(const std::string_view name , std::span<Value> value) override;
      virtual Opt<Value> OnCallMethod(const std::string_view name , Parameter* args , uint32_t argc) override;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_OBJECT_HPP
