/**
 * \file hosting/type.cpp
 **/
#include "hosting/type.hpp"

#include "core/utilities.hpp"

#include "hosting/attribute.hpp"
#include "hosting/field.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/method.hpp"
#include "hosting/native_string.hpp"
#include "hosting/property.hpp"
#include "hosting/type_cache.hpp"

namespace dotother {

  void Type::Init() {
    if (handle == -1 || initialized) {
      return;
    }

    int32_t count = 0;
    // methods
    {
      Interop().get_type_methods(handle, nullptr, &count);
      std::vector<int32_t> handles;
      handles.resize(count);
      Interop().get_type_methods(handle, handles.data(), &count);

      for (size_t i = 0; i < handles.size(); ++i) {
        methods.emplace_back(Method(handles[i]));
      }
    }

    /// fields
    {
      Interop().get_type_fields(handle, nullptr, &count);
      std::vector<int32_t> handles;
      handles.resize(count);
      Interop().get_type_fields(handle, handles.data(), &count);

      for (size_t i = 0; i < handles.size(); ++i) {
        fields.emplace_back(Field(handles[i]));
      }
    }

    // properties
    {
      Interop().get_type_properties(handle, nullptr, &count);
      std::vector<int32_t> handles;
      handles.resize(count);
      Interop().get_type_properties(handle, handles.data(), &count);

      for (size_t i = 0; i < handles.size(); ++i) {
        properties.emplace_back(Property(handles[i]));
      }
    }

    // attributes
    {
      Interop().get_type_attributes(handle, nullptr, &count);
      std::vector<int32_t> handles;
      handles.resize(count);
      Interop().get_type_attributes(handle, handles.data(), &count);

      for (size_t i = 0; i < handles.size(); ++i) {
        attributes.emplace_back(handles[i]);
      }
    }

    initialized = true;
    DOTOTHER_LOG(DO_STR("Type::Init: Initialized type: {}"), MessageLevel::TRACE, FullName());
  }

  Type& Type::BaseObject() {
    if (base_type == nullptr) {
      Type base;
      Interop().get_base_type(handle, &base.handle);
      base_type = TypeCache::Instance().CacheType(std::forward<Type>(base));
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

  std::vector<Method>& Type::Methods() {
    return methods;
  }

  std::vector<Field>& Type::Fields() {
    return fields;
  }

  std::vector<Property>& Type::Properties() {
    return properties;
  }

  std::vector<Attribute>& Type::Attributes() {
    return attributes;
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
      elt_type = TypeCache::Instance().CacheType(std::forward<Type>(elt));
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

  std::string FormatType(Type* t) {
    if (t == nullptr) {
      return "(null-type)";
    }
    std::string name = t->FullName();

    auto fields = t->Fields();
    auto properties = t->Properties();
    auto methods = t->Methods();
    auto attributes = t->Attributes();

    std::stringstream ss;
    std::stringstream fieldss;
    std::stringstream methodss;
    constexpr std::string_view template_str =
      R"(
      (
        Type : {},
            > Fields : {} 
              [{}]
            > Properties : {}
            > Methods : {} 
              [{}]
            > Attributes : {}
      )
      )";

    using namespace std::string_view_literals;

    fieldss << "\n";
    for (const auto& field : fields) {
      fieldss << fmt::format("                Field : {}\n"sv, field.GetName());
    }
    fieldss << "              ";

    methodss << "\n";
    for (auto& method : methods) {
      methodss << fmt::format("                Method : {}\n"sv, method.GetName());
    }
    methodss << "              ";

    // clang-format off
    ss << fmt::format(template_str, name, 
                      fields.size(), fieldss.str(), 
                      properties.size(), 
                      methods.size(), methodss.str(), 
                      attributes.size());
    // clang-format on
    return ss.str();
  }

}  // namespace dotother