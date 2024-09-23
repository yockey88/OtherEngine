/**
 * \file hosting/hosted_object.cpp
 **/
#include "hosting/hosted_object.hpp"

#include "hosting/native_string.hpp"
#include "hosting/interop_interface.hpp"

namespace dotother {

  void HostedObject::InvokeMethod(std::string_view method_name, const void** params, const ManagedType* types, size_t argc) {
    auto name = NString::New(method_name);
    Interop().invoke_method(managed_handle, name, params, types, static_cast<int32_t>(argc));
    NString::Free(name);
  }

  void HostedObject::InvokeReturningMethod(std::string_view method_name, const void** params, const ManagedType* types, 
                                            size_t argc, void* ret) {
    auto name = NString::New(method_name);
    Interop().invoke_method_ret(managed_handle, name, params, types, static_cast<int32_t>(argc), ret);
    NString::Free(name);
  }

  void HostedObject::WriteToField(const std::string_view name, void* value) {
    auto name_str = NString::New(name);
    Interop().set_field(managed_handle, name_str, value);
    NString::Free(name_str);
  }

  void HostedObject::ReadFromField(const std::string_view name, void* value) {
    auto name_str = NString::New(name);
    Interop().get_field(managed_handle, name_str, value);
    NString::Free(name_str);
  }

  void HostedObject::WriteToProperty(const std::string_view name, void* value) {
    auto name_str = NString::New(name);
    Interop().set_property(managed_handle, name_str, value);
    NString::Free(name_str);
  }

  void HostedObject::ReadFromProperty(const std::string_view name, void* value) {
    auto name_str = NString::New(name);
    Interop().get_property(managed_handle, name_str, value);
    NString::Free(name_str);
  }

} // namespace dotother
