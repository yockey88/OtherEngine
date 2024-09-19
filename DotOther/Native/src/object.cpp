/**
 * \file object.cpp
 **/
#include "object.hpp"

#include "native_string.hpp"
#include "interop_interface.hpp"

namespace dotother {

  void Object::InvokeMethodRetInternal(const std::string_view name, const void** args, ManagedType* param_types, size_t param_count, void* ret) {
    auto method_name = NString::New(name);
    interop::invoke_method_ret(this, method_name, args, param_types, param_count, ret);
  }

} // namespace dotother