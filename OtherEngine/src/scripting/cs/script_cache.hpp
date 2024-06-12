/**
 * \file scripting/cs/script_cache.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_CACHE_HPP
#define OTHER_ENGINE_SCRIPT_CACHE_HPP

#include <string_view>

#include <mono/metadata/class.h>
#include <mono/metadata/object-forward.h>

#include "core/uuid.hpp"
#include "core/value.hpp"
#include "scripting/script_field.hpp"
#include "scripting/cs/mono_utils.hpp"

namespace other {

  struct FieldData {
    ValueType vtype = ValueType::EMPTY;
    std::string name;
    UUID hash = 0;
    size_t size = 0;

    uint64_t flags = 0;

    bool is_property = false;
    
    inline bool HasFlag(FieldAccessFlag flag) const { return flags & (uint64_t)flag; }
    inline bool IsWritable() const { return (!HasFlag(FieldAccessFlag::READONLY) && HasFlag(FieldAccessFlag::PUBLIC)); }
    inline bool IsArray() const { return HasFlag(FieldAccessFlag::ARRAY); }
  };
  
  struct CsScriptField : ScriptField {
    FieldData& fdata;

    CsScriptField(FieldData& fdata)
      : fdata(fdata) {}
  };

  struct TypeData {
    std::string name;
    size_t size = 0;
    UUID id;
    MonoClass* asm_class = nullptr;
    bool is_custom = false;

    std::vector<UUID> methods;
    std::vector<UUID> fields;
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
      
  struct Cache {
    std::map<UUID , TypeData> class_data;
    std::map<UUID , FieldData> field_data;
    std::map<UUID , std::vector<MethodData>> method_data;
  };

  class CsScriptCache {
    public:
      static void InitializeCache();
      static void ShutdownCache();

      static UUID CacheAssembly(const std::string_view asm_name , MonoImage* asm_image);
      static void UnregisterAssembly(UUID id);

      static std::vector<ScriptField*> GetClassFields(UUID cache_id , MonoClass* klass , MonoObject* instance);

    private:
      static bool initialized;
      
      static void CacheAssembly(Cache& target_cache , const std::string_view asm_name , UUID id , MonoImage* asm_image , bool log_data = false);
      
      static void CacheClass(Cache& cache , std::string_view name , MonoClass* klass , bool log_data = false);

      static void StoreClassMethods(Cache& cache , TypeData& data);
      static void StoreClassFields(Cache& cache , TypeData& data);
      static void StoreClassProperties(Cache& cache , TypeData& data);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_CACHE_HPP
