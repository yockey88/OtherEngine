/**
 * \file scripting/cs/cs_script_cache.hpp
 **/
#ifndef OTHER_ENGINE_CS_SCRIPT_CACHE_HPP
#define OTHER_ENGINE_CS_SCRIPT_CACHE_HPP

#include <string_view>

#include <mono/metadata/class.h>
#include <mono/metadata/object-forward.h>
#include <mono/metadata/reflection.h>

#include "core/uuid.hpp"
#include "core/value.hpp"
#include "scripting/script_field.hpp"
#include "scripting/cs/mono_utils.hpp"

namespace other {

  struct CsAttributeData;

  struct CsFieldData {
    ValueType vtype = ValueType::EMPTY;
    std::string name;
    UUID hash = 0;
    size_t size = 0;

    uint64_t flags = 0;

    bool is_property = false;

    /// this should only be initialized by CacheClass -> StoreClassFields/StoreClassProperties
    union {
      struct {
        MonoProperty* asm_property;
        MonoMethod* asm_property_getter;
        MonoMethod* asm_property_setter;
      };
      MonoClassField* asm_field;
    };

    MonoArray* array = nullptr;

    MonoCustomAttrInfo* attr_info = nullptr;

    std::map<UUID , CsAttributeData> attributes;

    ~CsFieldData();
    
    inline bool HasFlag(FieldAccessFlag flag) const { return flags & (uint64_t)flag; }
    inline bool IsWritable() const { return (!HasFlag(FieldAccessFlag::READONLY) && HasFlag(FieldAccessFlag::PUBLIC)); }
    inline bool IsArray() const { return HasFlag(FieldAccessFlag::ARRAY); }
  };
  

  struct CsMethodData {
    std::string name;
    std::string full_name;
    UUID hash = 0;
    uint32_t flags = 0;
    uint32_t parameter_count = 0;

    bool is_static = false;
    bool is_virtual = false;

    MonoMethod* asm_method = nullptr;
    MonoCustomAttrInfo* attr_info = nullptr;
    
    std::map<UUID , CsAttributeData> attributes;

    ~CsMethodData();
  };
  
  struct CsTypeData {
    std::string name_space;
    std::string name;
    size_t size = 0;
    UUID id;
    bool is_custom = false;
    MonoClass* asm_class = nullptr;
    MonoCustomAttrInfo* attr_info = nullptr;

    std::map<UUID , CsAttributeData> attributes;
    std::map<UUID , CsFieldData> fields;
    std::map<UUID , CsMethodData> methods;

    ~CsTypeData();
  };
  
  struct CsAttributeData {
    std::string name;
    UUID hash = 0;
    size_t size = 0;

    MonoObject* attribute_instance = nullptr;
    CsTypeData* attr_type_data;
  };
      
  struct CsCache {
    std::map<UUID , CsFieldData> field_data;
    std::map<UUID , CsMethodData> method_data;
    std::map<UUID , CsTypeData> class_data;

    std::vector<ScriptField> GetClassFields(const std::string_view class_name);

    void CacheClass(const std::string_view name , MonoClass* klass);
    CsTypeData* GetClassData(UUID id);

    void Clear();

    private:
      void StoreClassMethods(CsTypeData& tdata);
      void StoreClassFields(CsTypeData& tdata);
      void StoreClassProperties(CsTypeData& tdata);
  };

} // namespace other

#endif // !OTHER_ENGINE_CS_SCRIPT_CACHE_HPP
