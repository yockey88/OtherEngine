/**
 * \file type_cache.cpp
 **/
#include "type_cache.hpp"

#include "utilities.hpp"
#include "type.hpp"

namespace dotother { 

  TypeCache& TypeCache::Instance() {
    static TypeCache instance;
    return instance;
  }

  Type* TypeCache::CacheType(Type&& type) {
    Type* t = &types.Insert(std::move(type)).second;

    std::string name = t->FullName();
    util::print(DO_STR("TypeCache::CacheType: Caching type: {}"), name);
    
    name_cache[name] = t;
    id_cache[t->handle] = t;
    return t;
  }

  Type* TypeCache::GetType(const std::string_view name) {
    auto name_str = std::string(name);
    bool contains = name_cache.contains(name_str);
    if (contains) {
      util::print(DO_STR("TypeCache::GetType: Found type: {}"), name_str);
    }

    return contains ?
      name_cache[name_str] : nullptr;
  }

  Type* TypeCache::GetType(int32_t id) {
    bool contains = id_cache.contains(id);
    if (contains) {
      util::print(DO_STR("TypeCache::GetType: Found type with ID: {}"), id);
    }
    
    return contains ? 
      id_cache[id] : nullptr;
  }

} // namespace dotother