/**
 * \file hosting/interop_interface.cpp
 **/
#include "hosting/interop_interface.hpp"

#include <refl/refl.hpp>

#include "core/defines.hpp"
#include "core/utilities.hpp"
#include "hosting/native_object.hpp"
#include "reflection/object_proxy.hpp"

namespace dotother {
namespace interface_bindings {
  
  bool FunctionTable::BoundToAsm() const {
    return (
      create_assembly_load_context != nullptr &&
      unload_assembly_load_context != nullptr && 
      load_assembly                != nullptr && 
      get_last_load_status         != nullptr && 
      get_assembly_name            != nullptr && 

      /// type functions
      get_asm_types          != nullptr &&
      get_type_id            != nullptr &&
      get_full_type_name     != nullptr &&
      get_asm_qualified_name != nullptr &&
      get_base_type          != nullptr &&
      get_type_size          != nullptr &&
      is_type_derived_from   != nullptr &&
      is_assignable_to       != nullptr &&
      is_assignable_from     != nullptr &&
      is_type_sz_array       != nullptr &&
      get_element_type       != nullptr &&
      get_type_methods       != nullptr &&
      get_type_fields        != nullptr &&
      get_type_properties    != nullptr &&
      has_type_attribute     != nullptr &&
      get_type_attributes    != nullptr &&
      get_type_managed_type  != nullptr &&

      /// field functions
      get_field_name          != nullptr &&
      get_field_type          != nullptr &&
      get_field_attributes    != nullptr &&
      get_field_accessibility != nullptr &&

      /// property functions
      get_property_name       != nullptr &&
      get_property_info       != nullptr &&
      get_property_attributes != nullptr &&

      /// attribute functions
      get_attr_value != nullptr &&
      get_attr_type  != nullptr &&
      
      /// method functions
      get_method_name          != nullptr &&
      get_method_return_type   != nullptr &&
      get_method_param_types   != nullptr &&
      get_method_attributes    != nullptr &&
      get_method_accessibility != nullptr &&

      set_internal_calls != nullptr &&
      set_internal_call != nullptr &&

      /// object functions
      create_object          != nullptr &&
      destroy_object         != nullptr &&
      invoke_method          != nullptr &&
      invoke_method_ret      != nullptr &&

      invoke_static_method     != nullptr &&
      invoke_static_method_ret != nullptr &&

      set_field != nullptr &&
      get_field != nullptr &&
      
      set_property != nullptr &&
      get_property != nullptr &&
      
      collect_garbage             != nullptr &&
      wait_for_pending_finalizers != nullptr
    );
  }

} // namespace interface_bindings

  InteropInterface* InteropInterface::instance = nullptr;

  InteropInterface& InteropInterface::Instance() {
    if (instance == nullptr) {
      instance = new InteropInterface();
    }
    return *instance;
  }
  
  using namespace std::string_view_literals;
  
  void InteropInterface::Unbind() {
    if (instance != nullptr) {
      delete instance;
      instance = nullptr;
    }

    util::print(DO_STR("InteropInterface unbound!"sv));
  }

  interface_bindings::FunctionTable& InteropInterface::FunctionTable() {
    return interop;
  }
  

  void InteropInterface::RegisterObject(uint64_t handle, NObject* object) {
    if (auto itr = registered_objects.find(handle); itr != registered_objects.end()) {
      util::print(DO_STR("Object {:#8x} already registered!"sv), handle);
      return;
    }

    util::print(DO_STR("Registering object {:#8x}"sv), handle);
    registered_objects[handle] = object;
  }

  void InteropInterface::InvokeNativeFunction(uint64_t obj_handle , const std::string_view method_name) {
    util::print(DO_STR("Invoking {} on {:#8x}"sv), method_name, obj_handle);
    if (auto itr = registered_objects.find(obj_handle); itr == registered_objects.end()) {
      util::print(DO_STR("Object {:#8x} not found!"sv), obj_handle);
      return;
    }

    NObject& obj = *registered_objects[obj_handle];
    obj.proxy->InvokeMethod(method_name);
  }

} // namespace dotother
