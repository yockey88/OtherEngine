/**
 * \file hosting/method.cpp
 **/
#include "hosting/method.hpp"

#include "hosting/attribute.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/type.hpp"
#include "hosting/type_cache.hpp"

namespace dotother {

  Method::Method(int32_t handle)
      : handle(handle) {
    // ParamTypes();
  }

  NString Method::GetName() const {
    return Interop().get_method_name(handle);
  }

  Type& Method::GetReturnType() {
    if (ret_type == nullptr) {
      Type ret_t;
      Interop().get_method_return_type(handle, &ret_t.handle);
      ret_type = TypeCache::Instance().CacheType(std::forward<Type>(ret_t));
    }
    return *ret_type;
  }

  const std::vector<Type*>& Method::ParamTypes() {
    if (param_types.size() > 0) {
      return param_types;
    }

    int32_t count = 0;
    Interop().get_method_param_types(handle, nullptr, &count);

    if (count == 0) {
      return param_types;
    }

    std::vector<int32_t> types;
    types.resize(count);
    Interop().get_method_param_types(handle, types.data(), &count);

    param_types.resize(count);
    for (size_t i = 0; i < types.size(); ++i) {
      Type t;
      t.handle = types[i];
      param_types[i] = TypeCache::Instance().CacheType(std::forward<Type>(t));
    }

    return param_types;
  }

  size_t Method::Arity() const {
    return param_types.size();
  }

  TypeAccessibility Method::Accessibility() const {
    return Interop().get_method_accessibility(handle);
  }

  std::vector<Attribute> Method::Attributes() const {
    int32_t count = 0;
    Interop().get_method_attributes(handle, nullptr, &count);

    std::vector<int32_t> handles{ count };
    Interop().get_method_attributes(handle, handles.data(), &count);

    std::vector<Attribute> res;
    for (size_t i = 0; i < handles.size(); ++i) {
      res.emplace_back(handles[i]);
    }

    return res;
  }

}  // namespace dotother
