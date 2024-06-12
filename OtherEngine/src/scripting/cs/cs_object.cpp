/**
 * \file scripting/cs/cs_object.cpp
*/
#include "scripting/cs/cs_object.hpp"

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

  void CsObject::InitializeScriptFields() {}

  Opt<Value> CsObject::CallMethod(const std::string& name , Parameter* args , uint32_t argc)  {
    UUID id = FNV(name);

    if (other_methods.find(id) != other_methods.end()) {
      return CallMonoMethod(other_methods[id] , argc , args);
    }
    
    MonoMethod* method = mono_class_get_method_from_name(klass , name.c_str() , 0);
    if (method == nullptr) {
      OE_ERROR("Mono Script Object [{}] method [{}] not found" , script_name , name);
      OE_WARN("Mono Script Object [{}] invalid" , script_name);
      is_corrupt = true;
      return std::nullopt;
    }
    
    /// cache the method
    other_methods[id] = method;
    return CallMonoMethod(method , argc , args);
  }

  void CsObject::Initialize() {
    if (initialize_method != nullptr) {
      CallMonoMethod(initialize_method);
    }

    is_initialized = true;
  }

  void CsObject::Update(float dt) {
    if (update_method != nullptr) {
      Parameter arg = {
        .handle = &dt ,
        .type = ValueType::FLOAT ,
      };
      CallMonoMethod(update_method , 1 , &arg);
    }
  }

  void CsObject::Render() {
    if (render_method != nullptr) {
      CallMonoMethod(render_method);
    }
  }

  void CsObject::RenderUI() {
    if (render_ui_method != nullptr) {
      CallMonoMethod(render_ui_method);
    }
  }

  void CsObject::Shutdown() {
    if (shutdown_method != nullptr) {
      CallMonoMethod(shutdown_method);
    }

    is_initialized = false;
  }
  
  Opt<Value> CsObject::CallMonoMethod(MonoMethod* method , uint32_t argc , Parameter* args) {
    if (is_corrupt) {
      return std::nullopt;
    }

    if (instance == nullptr) {
      OE_ERROR("CsObject::CallMethod: instance is nullptr");
      is_corrupt = true;
      return std::nullopt;
    }

    if (method == nullptr) {
      return std::nullopt;
    }

    std::vector<ParamHandle> params(argc);
    for (uint32_t i = 0; i < argc; ++i) {
      params[i] = args[i].handle;
    }

    MonoObject* exception = nullptr;
    MonoObject* ret_val = mono_runtime_invoke(method , instance , params.data() , &exception);
    if (exception) {
      OE_ERROR("CsObject::CallMethod: exception");
      is_corrupt = true; 
      return std::nullopt;
    }

    if (ret_val == nullptr) {
      return std::nullopt;
    }

    return MonoObjectToValue(ret_val);
  }

} // namespace other
