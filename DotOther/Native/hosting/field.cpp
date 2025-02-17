/**
 * \file hosting/field.cpp
 **/
#include "hosting/field.hpp"

#include "hosting/attribute.hpp"
#include "hosting/interop_interface.hpp"
#include "hosting/type.hpp"
#include "hosting/type_cache.hpp"

namespace dotother {

  Field::Field(uint32_t handle) {
    this->handle = handle;
  }

  NString Field::GetName() const {
    return Interop().get_field_name(handle);
  }

  Type& Field::GetType() {
    if (type == nullptr) {
      Type type;
      Interop().get_field_type(handle, &type.handle);
      this->type = TypeCache::Instance().CacheType(std::forward<Type>(type));

      if (this->type == nullptr) {
        DOTOTHER_LOG(DO_STR("Field::GetType: Failed to cache field type"), MessageLevel::ERR);
        static Type null_type(-1);
        return null_type;
      }
    }
    return *type;
  }

  TypeAccessibility Field::Accessibility() const {
    return Interop().get_field_accessibility(handle);
  }

  std::vector<Attribute> Field::Attributes() const {
    int32_t count = 0;
    Interop().get_field_attributes(handle, nullptr, &count);

    std::vector<int32_t> raw_attrs;
    raw_attrs.resize(count);
    Interop().get_field_attributes(handle, raw_attrs.data(), &count);

    std::vector<Attribute> attrs;
    for (auto attr : raw_attrs) {
      attrs.emplace_back(attr);
    }

    return attrs;
  }

}  // namespace dotother
