/**
 * \file scripting/cs/cs_object.cpp
*/
#include "cs_object.hpp"

#include "core/logger.hpp"

namespace other {

  void CsObject::InitializeScriptMethods() {
    initialize_method = mono_class_get_method_from_name(klass , "Initialize" , 0);
    update_method = mono_class_get_method_from_name(klass , "Update" , 1);
    render_method = mono_class_get_method_from_name(klass , "Render" , 0);
    render_ui_method = mono_class_get_method_from_name(klass , "RenderUI" , 0);
    shutdown_method = mono_class_get_method_from_name(klass , "Shutdown" , 0);
  }

   Opt<Value> CsObject::CallMethod(const std::string& name , ParamHandle* args) {
    UUID id = FNV(name);

    if (other_methods.find(id) != other_methods.end()) {
      CallMethod(other_methods[id] , args);
    }

    MonoMethod* method = mono_class_get_method_from_name(klass , name.c_str() , 0);
    if (method == nullptr) {
      OE_ERROR("Mono Script Object [{}] method [{}] not found" , script_name , name);
      OE_WARN("Mono Script Object [{}] invalid" , script_name);
      class_dirty = true;
      return std::nullopt;
    }

    other_methods[id] = method;
    return CallMethod(method , args);
  }

  void CsObject::Initialize() {
    if (initialize_method) {
      CallMethod(initialize_method , nullptr);
    }
  }

  void CsObject::Update(float dt) {
    if (update_method) {
      ParamHandle args[1] = { &dt };
      CallMethod(update_method , args);
    }
  }

  void CsObject::Render() {
    if (render_method) {
      CallMethod(render_method , nullptr);
    }
  }

  void CsObject::RenderUI() {
    if (render_ui_method) {
      CallMethod(render_ui_method , nullptr);
    }
  }

  void CsObject::Shutdown() {
    if (shutdown_method) {
      CallMethod(shutdown_method , nullptr);
    }
  }

  Opt<Value> CsObject::CallMethod(MonoMethod* method , ParamHandle* args) {
    if (instance == nullptr) {
      OE_ERROR("CsObject::CallMethod: instance is nullptr");
      class_dirty = true;
      return std::nullopt;
    }

    MonoObject* exception = nullptr;
    mono_runtime_invoke(method , instance , args , &exception);
    if (exception) {
      OE_ERROR("CsObject::CallMethod: exception");
      class_dirty = true; 
      return std::nullopt;
    }

    return std::nullopt;
  }

} // namespace other
