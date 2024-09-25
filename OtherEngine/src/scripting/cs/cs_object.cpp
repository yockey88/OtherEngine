/**
 * \file scripting/cs/cs_object.cpp
*/
#include "scripting/cs/cs_object.hpp"

#include "core/logger.hpp"

namespace other {

  void CsObject::InitializeScriptMethods() {
  }

  void CsObject::InitializeScriptFields() {
  } 

  void CsObject::UpdateNativeFields() {
  }
      
  Opt<Value> CsObject::GetField(const std::string& name) {
    return std::nullopt;
  }

  void CsObject::SetField(const std::string& name , const Value& value) {
  }
    
  void CsObject::OnBehaviorLoad() {
  }

  void CsObject::Initialize() {
  }
  
  void CsObject::Shutdown() {
  }
  
  void CsObject::OnBehaviorUnload() {
  }

  void CsObject::Start() {
  }

  void CsObject::Stop() {
  }
  
  void CsObject::EarlyUpdate(float dt) {
  }
  
  void CsObject::Update(float dt) {
  }

  void CsObject::LateUpdate(float dt) {
  }

  void CsObject::Render() {
  }

  void CsObject::RenderUI() {
  }

  void CsObject::OnSetEntityId() {
  }
  
  Opt<Value> CsObject::OnCallMethod(const std::string_view name , std::span<Value> args) {
    return std::nullopt;
  }
  
  Opt<Value> CsObject::OnCallMethod(const std::string_view name , Parameter* args , uint32_t argc)  {
    return std::nullopt;
  }

} // namespace other
