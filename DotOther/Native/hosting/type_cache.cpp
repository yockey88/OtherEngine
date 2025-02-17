/**
 * \file hosting/type_cache.cpp
 **/
#include "hosting/type_cache.hpp"

#include <sstream>

#include "core/utilities.hpp"

#include "hosting/attribute.hpp"
#include "hosting/field.hpp"
#include "hosting/method.hpp"
#include "hosting/property.hpp"
#include "hosting/type.hpp"

namespace dotother {

  TypeCache& TypeCache::Instance() {
    static TypeCache instance;
    return instance;
  }

  Type* TypeCache::CacheType(Type&& type) {
    Type* t = &types.Insert(std::move(type)).second;
    if (t == nullptr) {
      DOTOTHER_LOG(DO_STR("TypeCache::CacheType: Failed to cache type"), MessageLevel::ERR);
      return nullptr;
    }
    t->Init();

    DOTOTHER_LOG(DO_STR("TypeCache::CacheType: Caching type {}"), MessageLevel::TRACE, type.FullName());  // , FormatType(t));

    std::string name = t->FullName();
    name_cache[name] = t;
    id_cache[t->handle] = t;
    return t;
  }

  Type* TypeCache::GetType(const std::string_view name) {
    auto name_str = std::string(name);
    bool contains = name_cache.contains(name_str);
    if (contains) {
      Type* t = name_cache[name_str];

      DOTOTHER_LOG(DO_STR("TypeCache::GetType: Found type"), MessageLevel::TRACE);  // , FormatType(t));
      return t;
    }

    return nullptr;
  }

  Type* TypeCache::GetType(int32_t id) {
    bool contains = id_cache.contains(id);
    if (contains) {
      Type* t = id_cache[id];
      DOTOTHER_LOG(DO_STR("TypeCache::GetType: Found type with ID [{}]"), MessageLevel::TRACE, id);  // , FormatType(t));
      return t;
    }
    DOTOTHER_LOG(DO_STR("TypeCache::GetType: Type not found with ID [{}]"), MessageLevel::ERR, id);

    return nullptr;
  }

}  // namespace dotother