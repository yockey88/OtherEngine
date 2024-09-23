/**
 * \file hosting/type.cpp
 **/
#include "hosting/type.hpp"

#include "core/utilities.hpp"
#include "hosting/native_string.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/type_cache.hpp"
#include "hosting/field.hpp"
#include "hosting/method.hpp"
#include "hosting/property.hpp"
#include "hosting/attribute.hpp"

namespace dotother {

  Type& Type::BaseObject() {
    if (base_type == nullptr) {
      Type base;
      Interop().get_base_type(handle, &base.handle);
      base_type = TypeCache::Instance().CacheType(std::move(base));
    }

    return *base_type;
  }

  int32_t Type::TypeSize() {
    return Interop().get_type_size(handle);
  }

  bool Type::DerivedFrom(const Type& type) {
    return Interop().is_type_derived_from(handle, type.handle);
  }

  bool Type::AssignableTo(const Type& type) {
    return Interop().is_assignable_to(handle, type.handle);
  }

  bool Type::AssignableFrom(const Type& type) {
    return Interop().is_assignable_from(handle, type.handle);
  }

  std::vector<Method> Type::Methods() { 
    int32_t count = 0;
    Interop().get_type_methods(handle, nullptr, &count);
    if (count == 0) {
      return {};
    }

    std::vector<int32_t> handles;
    handles.resize(count);
    Interop().get_type_methods(handle, handles.data(), &count);
    
    std::vector<Method> res;
    res.resize(handles.size());
    for (size_t i = 0; i < handles.size(); ++i) {
      res[i].handle = handles[i];
    }

    util::print(DO_STR("{} : Loaded {} methods ({})"), FullName(), handles.size(), count);
    return res;
  }
  
  std::vector<Field> Type::Fields() { 
    int32_t count = 0;
    Interop().get_type_fields(handle, nullptr, &count);

    std::vector<int32_t> handles{ count };
    Interop().get_type_fields(handle, handles.data(), &count);

    std::vector<Field> res{ handles.size() };
    for (size_t i = 0; i < handles.size(); ++i) {
      res[i].handle = handles[i];
    }
    return res;
  }
  
  std::vector<Property> Type::Properties() {
    int32_t count = 0;
    Interop().get_type_properties(handle, nullptr, &count);

    std::vector<int32_t> handles{ count };
    Interop().get_type_properties(handle, handles.data(), &count);

    std::vector<Property> res{ handles.size() };
    for (size_t i = 0; i < handles.size(); ++i) {
      res[i].handle = handles[i];
    }
    return res;
  }
  
  std::vector<Attribute> Type::Attributes() {
    int32_t count = 0;
    Interop().get_type_attributes(handle, nullptr, &count);

    std::vector<int32_t> handles{ count };
    Interop().get_type_attributes(handle, handles.data(), &count);

    std::vector<Attribute> res{ handles.size() };
    for (size_t i = 0; i < handles.size(); ++i) {
      res[i].handle = handles[i];
    }
    return res;
  }

  bool Type::HasAttribute(const Type& type) {
    return Interop().has_type_attribute(handle, type.handle);
  }

  ManagedType Type::GetManagedType() {
    return Interop().get_type_managed_type(handle);
  }

  bool Type::IsArray() {
    return Interop().is_type_sz_array(handle);
  }
  
  Type& Type::GetEltType() {
    if (elt_type == nullptr) {
      Type elt;
      Interop().get_element_type(handle, &elt.handle);
      elt_type = TypeCache::Instance().CacheType(std::move(elt));
    }

    return *elt_type;
  }

  bool Type::operator==(const Type& other) {
    return handle == other.handle;
  }

  Type::operator bool() {
    return handle != -1;
  }

  NString Type::FullName() {
    return Interop().get_full_type_name(handle);
  }
  
  HostedObject Type::New(const void** argv, const ManagedType* arg_ts, size_t argc) {
    HostedObject res;
    res.managed_handle = Interop().create_object(handle, false, argv, arg_ts, argc);
    res.type = this;
    return res;
  }

} // namespace dotother