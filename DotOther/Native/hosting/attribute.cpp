/**
 * \file hosting/attribute.hpp
 **/
#include "hosting/attribute.hpp"

#include "core/utilities.hpp"

#include "hosting/type.hpp"
#include "hosting/type_cache.hpp"

namespace dotother {

  Attribute::Attribute(uint32_t hash) {
    handle = hash;
  }

  Type& Attribute::GetType() {
    Type* type = TypeCache::Instance().CacheType(Type(handle));
    if (type == nullptr) {
      DOTOTHER_LOG(DO_STR("Attribute::GetType: Failed to cache attribute type"), MessageLevel::ERR);
      static Type null_type(-1);
      return null_type;
    }
    return *type;
  }

}  // namespace dotother