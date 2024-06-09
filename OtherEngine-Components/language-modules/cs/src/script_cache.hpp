/**
 * \file script_cache.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_CACHE_HPP
#define OTHER_ENGINE_SCRIPT_CACHE_HPP

#include <string_view>

#include <mono/metadata/class.h>
#include <mono/metadata/object-forward.h>

#include "core/uuid.hpp"

namespace other {

  struct TypeData {
    std::string name;
    size_t size = 0;
    UUID id;
    MonoClass* asm_class = nullptr;
  };

  struct MethodData {
    TypeData* owning_type = nullptr; 
    std::string name;
    UUID hash = 0;
    uint32_t flags = 0;
    uint32_t parameter_count = 0;

    bool is_static = false;
    bool is_virtual = false;

    MonoMethod* asm_method = nullptr;
  };

  class CsScriptCache {
    public:
      static void InitializeCache();
      static void ShutdownCache();

      static void CacheClass(const std::string_view name , MonoClass* klass);

    private:
      static bool initialized;

      static void StoreClassMethods(TypeData& data);
      static void StoreClassFields(TypeData& data);
      static void StoreClassProperties(TypeData& data);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_CACHE_HPP
