/**
 * \file object.cpp
 **/
#include "object.hpp"

#include "native_string.hpp"
#include "interop_interface.hpp"

namespace dotother {

  void Object::InvokeMethod(std::string_view method_name, const void** params, const ManagedType* types, size_t argc) {
    auto name = NString::New(method_name);
    Interop().invoke_method(managed_handle, name, params, types, static_cast<int32_t>(argc));
    NString::Free(name);
  }

  void Object::InvokeReturningMethod(std::string_view method_name, const void** params, const ManagedType* types, 
                                            size_t argc, void* ret) {
    auto name = NString::New(method_name);
    Interop().invoke_method_ret(managed_handle, name, params, types, static_cast<int32_t>(argc), ret);
    NString::Free(name);
  }

  void Object::WriteToField(const std::string_view name, void* value) {
    auto name_str = NString::New(name);
    Interop().set_field(managed_handle, name_str, value);
    NString::Free(name_str);
  }

  void Object::ReadFromField(const std::string_view name, void* value) {
    auto name_str = NString::New(name);
    Interop().get_field(managed_handle, name_str, value);
    NString::Free(name_str);
  }

  void Object::WriteToProperty(const std::string_view name, void* value) {
    auto name_str = NString::New(name);
    Interop().set_property(managed_handle, name_str, value);
    NString::Free(name_str);
  }

  void Object::ReadFromProperty(const std::string_view name, void* value) {
    auto name_str = NString::New(name);
    Interop().get_property(managed_handle, name_str, value);
    NString::Free(name_str);
  }

} // namespace dotother
