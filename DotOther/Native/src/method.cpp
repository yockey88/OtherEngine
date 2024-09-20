/**
 * \file method.cpp
 **/
#include "method.hpp"

#include "type_cache.hpp"
#include "interop_interface.hpp"
#include "type.hpp"
#include "attribute.hpp"

namespace dotother {

  NString Method::GetName() const {
    return Interop().get_method_name(handle); 
  }

  Type& Method::GetReturnType() {
    if (ret_type == nullptr) {
      Type ret_t;
      Interop().get_method_return_type(handle, &ret_t.handle);
      ret_type = TypeCache::Instance().CacheType(std::move(ret_t));
    }
    return *ret_type;
  }
      
  const std::vector<Type*>& Method::ParamTypes() {
    if (param_types.size() > 0) {
      return param_types;
    } 

    int32_t count = 0;
    Interop().get_method_param_types(handle, nullptr, &count);

    std::vector<int32_t> types{ count };
    Interop().get_method_param_types(handle, types.data(), &count);

    param_types.resize(types.size());
    for (size_t i = 0; i < types.size(); ++i) {
      Type t;
      t.handle = types[i];
      param_types[i] = TypeCache::Instance().CacheType(std::move(t));
    }

    return param_types;
  }

  TypeAccessibility Method::Accessibility() const {
    return Interop().get_method_accessibility(handle);
  }

  std::vector<Attribute> Method::Attributes() const {
    int32_t count = 0;
    Interop().get_method_attributes(handle, nullptr, &count);

    std::vector<int32_t> handles{ count };
    Interop().get_method_attributes(handle, handles.data(), &count);

    std::vector<Attribute> res{ handles.size() };
    for (size_t i = 0; i < handles.size(); ++i) {
      res[i].handle = handles[i];
    }

    return res;
  }

} // namespace dotother
