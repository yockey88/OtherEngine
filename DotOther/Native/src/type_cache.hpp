/**
 * \file type_cached.hpp
 **/
#ifndef DOTOTHER_TYPE_CACHED_HPP
#define DOTOTHER_TYPE_CACHED_HPP

#include <string>
#include <string_view>
#include <unordered_map>

#include "stable_vector.hpp"

namespace dotother {

  class Type;

  class TypeCache {
    public:
      static TypeCache& Instance();

      Type* CacheType(Type&& type);
      Type* GetType(const std::string_view name);
      Type* GetType(int32_t name);

    private:
      TypeCache() = default;
      ~TypeCache() = default;

      TypeCache(TypeCache&&) = delete;
      TypeCache(const TypeCache&) = delete;
      TypeCache& operator=(TypeCache&&) = delete;
      TypeCache& operator=(const TypeCache&) = delete;

      StableVector<Type> types;
      std::unordered_map<std::string, Type*> name_cache;
      std::unordered_map<int32_t, Type*> id_cache;
  };

} // namespace dotother

#endif // !DOTOTHER_TYPE_CACHED_HPP