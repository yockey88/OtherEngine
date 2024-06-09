/**
 * \file scripting/cs/cs_object.cpp
*/
#include "cs_object.hpp"

#include "core/logger.hpp"

#include "mono_utils.hpp"

namespace other {

  void CsObject::InitializeScriptMethods() {
    initialize_method = mono_class_get_method_from_name(klass , "OnInitialize" , 0);
    update_method = mono_class_get_method_from_name(klass , "Update" , 1);
    render_method = mono_class_get_method_from_name(klass , "Render" , 0);
    render_ui_method = mono_class_get_method_from_name(klass , "RenderUI" , 0);
    shutdown_method = mono_class_get_method_from_name(klass , "OnShutdown" , 0);
  }

  void CsObject::CallMethod(Value* ret , const std::string& name , ParamHandle* args) {
    UUID id = FNV(name);

    if (other_methods.find(id) != other_methods.end()) {
      Opt<Value> return_val = CallMethod(other_methods[id] , args);
      if (return_val.has_value()) {
        *ret = *return_val;
      }
      return;
    }
    
    MonoMethod* method = mono_class_get_method_from_name(klass , name.c_str() , 0);
    if (method == nullptr) {
      OE_ERROR("Mono Script Object [{}] method [{}] not found" , script_name , name);
      OE_WARN("Mono Script Object [{}] invalid" , script_name);
      class_dirty = true;
      // *ret = std::nullopt;
      return;
    }

    /// cache the method
    other_methods[id] = method;
    Opt<Value> return_val = CallMethod(method , args);
    if (return_val.has_value()) {
      *ret = *return_val;
    }

    return;
  }

  void CsObject::GetProperty(Value* val , const std::string& name) const {
  }
  
  void CsObject::SetProperty(const Value& value) {
  }

  void CsObject::Initialize() {
    if (initialize_method != nullptr) {
      CallMethod(initialize_method , nullptr);
    }
  }

  void CsObject::Update(float dt) {
    if (update_method != nullptr) {
      ParamHandle args[1] = { &dt };
      CallMethod(update_method , args);
    }
  }

  void CsObject::Render() {
    if (render_method != nullptr) {
      CallMethod(render_method , nullptr);
    }
  }

  void CsObject::RenderUI() {
    if (render_ui_method != nullptr) {
      CallMethod(render_ui_method , nullptr);
    }
  }

  void CsObject::Shutdown() {
    if (shutdown_method != nullptr) {
      CallMethod(shutdown_method , nullptr);
    }
  }

  Opt<Value> CsObject::CallMethod(MonoMethod* method , ParamHandle* args) {
    if (class_dirty) {
      return std::nullopt;
    }

    if (instance == nullptr) {
      OE_ERROR("CsObject::CallMethod: instance is nullptr");
      class_dirty = true;
      return std::nullopt;
    }

    if (method == nullptr) {
      return std::nullopt;
    }

    MonoObject* exception = nullptr;
    MonoObject* ret_val = mono_runtime_invoke(method , instance , args , &exception);
    if (exception) {
      OE_ERROR("CsObject::CallMethod: exception");
      class_dirty = true; 
      return std::nullopt;
    }

    if (ret_val == nullptr) {
      return std::nullopt;
    }

    return MonoObjectToValue(ret_val);
  }

} // namespace other
