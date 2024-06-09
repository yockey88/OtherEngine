/**
 * \file scripting/cs/script_core_cache.cpp
 **/
#include "scripting/cs/script_cache.hpp"

#include <iterator>
#include <map>

#include <mono/metadata/blob.h>
#include <mono/metadata/class.h>
#include <mono/metadata/image.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/row-indexes.h>

#include "core/defines.hpp"
#include "core/value.hpp"
#include "scripting/cs/mono_utils.hpp"

namespace other {

  struct AssemblyCaches {
    size_t loaded_asms = 0;
    std::map<UUID , Cache> asm_caches;
  };

  static UUID core_id = 0;
  static Cache* core_cache = nullptr;
  static AssemblyCaches* asm_caches = nullptr;

  bool CsScriptCache::initialized = false;

  void CsScriptCache::InitializeCache() {
    if (initialized) {
      return;
    } 
    
    core_cache = new Cache; 
    asm_caches = new AssemblyCaches;

    CacheAssembly(*core_cache , "MSCORLIB" , FNV("MSCORLIB") , mono_get_corlib());

#define CACHE_MSCORLIB_CLASS(name) \
    CacheClass(*core_cache , "System::" ##name , mono_class_from_name(mono_get_corlib() , "System" , name))

    // CACHE_MSCORLIB_CLASS("Object");
    // CACHE_MSCORLIB_CLASS("ValueType");
    // CACHE_MSCORLIB_CLASS("Enum");
    // CACHE_MSCORLIB_CLASS("String");
    // CACHE_MSCORLIB_CLASS("Boolean");
    // CACHE_MSCORLIB_CLASS("Char");
    // CACHE_MSCORLIB_CLASS("SByte");
    // CACHE_MSCORLIB_CLASS("Byte");
    // CACHE_MSCORLIB_CLASS("Int16");
    // CACHE_MSCORLIB_CLASS("UInt16");
    // CACHE_MSCORLIB_CLASS("Int32");
    // CACHE_MSCORLIB_CLASS("UInt32");
    // CACHE_MSCORLIB_CLASS("Int64");
    // CACHE_MSCORLIB_CLASS("UInt64");
    // CACHE_MSCORLIB_CLASS("Single");
    // CACHE_MSCORLIB_CLASS("Double");
    // CACHE_MSCORLIB_CLASS("Void");

#undef CACHE_MSCORLIB_CLASS
    
    initialized = true;
  }
      
  void CsScriptCache::ShutdownCache() {
    if (!initialized) {
      return;
    }

    initialized = false;

    core_cache->class_data.clear();
    core_cache->field_data.clear();
    core_cache->method_data.clear();
    delete core_cache;
  }
        
  UUID CsScriptCache::CacheAssembly(const std::string_view asm_name , MonoImage* asm_image) {
    if (asm_image == nullptr) {
      OE_ERROR("Attempting to cache null C# assembly image");
      return 0;
    }
    
    UUID hash = FNV(asm_name);
    if (asm_caches->asm_caches.find(hash) != asm_caches->asm_caches.end()) {
      OE_ERROR("Attempting to re-cache cached C# assembly {}" , asm_name);
      return hash;
    }
    
    auto& cache = asm_caches->asm_caches[hash] = Cache{};
    CacheAssembly(cache , asm_name , hash , asm_image , true);

    return hash; 
  }
  
  void CsScriptCache::UnregisterAssembly(UUID id) {
    if (asm_caches->asm_caches.find(id) == asm_caches->asm_caches.end()) {
      OE_ERROR("Attempting to unload unregistered assembly {}" , id);
      return;
    }

    auto& cache = asm_caches->asm_caches[id];
    cache.class_data.clear();
    cache.field_data.clear();
    cache.method_data.clear();
    asm_caches->asm_caches.erase(asm_caches->asm_caches.find(id));

    OE_DEBUG("Unregistered C# assembly");
  }
  
  void CsScriptCache::CacheAssembly(Cache& target_cache , const std::string_view asm_name , UUID id , MonoImage* asm_image , bool log_data) {
    if (asm_image == nullptr) {
      OE_ERROR("Attempting to cache null C# assembly image");
      return;
    }

    const MonoTableInfo* table = mono_image_get_table_info(asm_image , MONO_TABLE_TYPEDEF);
    uint32_t rows = mono_table_info_get_rows(table);

    for (uint32_t i = 0; i < rows; ++i) {
      uint32_t cols[MONO_TYPEDEF_SIZE];
      mono_metadata_decode_row(table , i , cols , MONO_TYPEDEF_SIZE);

      const char* name_space  = mono_metadata_string_heap(asm_image , cols[MONO_TYPEDEF_NAMESPACE]);
      const char* name = mono_metadata_string_heap(asm_image , cols[MONO_TYPEDEF_NAME]);

      std::string name_str{ name };

      if (name_str == "<Module>") {
        continue;
      }
      
      std::string full_name{ name_space };
      full_name += "::" + name_str;

      MonoClass* klass = mono_class_from_name(asm_image , name_space , name);
      if (klass == nullptr) {
        if (log_data) {
          OE_WARN("Failed to cache class {} in C# assembly {}" , full_name , asm_name);
        }
        continue;
      }

      CacheClass(target_cache , name_str , klass , log_data); 
    }
  }
        
  void CsScriptCache::CacheClass(Cache& cache , const std::string_view name , MonoClass* klass , bool log_data) {
    if (klass == nullptr) {
      OE_WARN("Attempting to cache null C# class {}" , name);
      return;
    } else if (log_data) {
      OE_INFO(" > Caching C# class {}" , name);
    }

    TypeData data{};
    data.name = name;

    data.id = FNV(name);

    uint32_t align = 0;
    data.size = mono_class_value_size(klass , &align);
    data.asm_class = klass;

    TypeData& td = cache.class_data[data.id] = data;

    if (name.find("Other.") != std::string::npos) {
      StoreClassMethods(cache , td);
      StoreClassFields(cache ,td);
      StoreClassProperties(cache , td);
    }
  }

  void CsScriptCache::StoreClassMethods(Cache& cache , TypeData& data) {
    if (data.asm_class == nullptr) {
      return;
    }

    MonoMethod* method = nullptr;
    MethodHandle iter = nullptr;

    do {
      method = mono_class_get_methods(data.asm_class , &iter);
      if (method == nullptr) {
        break;
      }

      MonoMethodSignature* sig = mono_method_signature(method);
      MethodData method_data;

      method_data.owning_type = &data;

      method_data.name = std::string{ mono_method_full_name(method , 0) };

      std::string case_ins;
      std::transform(method_data.name.begin() , method_data.name.end() , std::back_inserter(case_ins) , ::toupper);

      method_data.hash = FNV(case_ins);
      method_data.flags = mono_method_get_flags(method , nullptr);

      method_data.is_static = method_data.flags & MONO_METHOD_ATTR_STATIC;
      method_data.is_virtual = method_data.flags & MONO_METHOD_ATTR_VIRTUAL;
      method_data.parameter_count = mono_signature_get_param_count(sig);
      method_data.asm_method = method;

      cache.method_data[method_data.hash].push_back(method_data);
      data.methods.push_back(method_data.hash);

    } while (method != nullptr);
  }

  void CsScriptCache::StoreClassFields(Cache& cache , TypeData& data) {
    if (data.asm_class == nullptr) {
      return;
    }

    std::string name = std::string{ mono_class_get_name(data.asm_class) };
    std::string name_space = std::string{ mono_class_get_namespace(data.asm_class) };

    MonoClass* curr_class = data.asm_class;
    while (curr_class != nullptr) {
      std::string class_name{ mono_class_get_name(curr_class) };
      std::string class_namespace{ mono_class_get_namespace(curr_class) }; 

      if (class_namespace.find("Other") != std::string::npos && class_name.find("Entity") != std::string::npos) {
        curr_class = nullptr;
        continue;
      }

      MonoClassField* field = nullptr;
      FieldHandle iter = nullptr;

      do {
        field = mono_class_get_fields(curr_class , &iter);
        if (field == nullptr) {
          break;
        }

        std::string name{ mono_field_get_name(field) };

        if (name.find("k__BackingField") != std::string::npos) {
          continue;
        }

        MonoType* mtype = mono_field_get_type(field);
        ValueType ftype = ValueTypeFromMonoType(mtype);

        if (ftype == ValueType::EMPTY) {
          continue;
        }

        // MonoCustomAttrInfo* attr = mono_custom_attrs_from_field(curr_class , field);

        std::string hash_str = data.name + ":" + name;
        UUID id = FNV(hash_str);

        int32_t type_encoding = mono_type_get_type(mtype);

        FieldData sf = cache.field_data[id];

        sf.name = std::string{ mono_field_get_name(field) };
        sf.hash = id;
        sf.vtype = ftype;
        sf.is_property = false;

        if (type_encoding == MONO_TYPE_ARRAY || type_encoding == MONO_TYPE_SZARRAY) {
          sf.flags |= static_cast<uint64_t>(FieldAccessFlag::ARRAY);          
        }

        uint32_t visibility = mono_field_get_flags(field) * MONO_FIELD_ATTR_FIELD_ACCESS_MASK;
        switch (visibility) {
          case MONO_FIELD_ATTR_PUBLIC:
            sf.flags &= ~(uint64_t)FieldAccessFlag::PROTECTED;
            sf.flags &= ~(uint64_t)FieldAccessFlag::PRIVATE;
            sf.flags &= ~(uint64_t)FieldAccessFlag::INTERNAL;
            sf.flags |= (uint64_t)FieldAccessFlag::PUBLIC;
          break;
          case MONO_FIELD_ATTR_FAMILY:
            sf.flags &= ~(uint64_t)FieldAccessFlag::PUBLIC;
            sf.flags &= ~(uint64_t)FieldAccessFlag::PRIVATE;
            sf.flags &= ~(uint64_t)FieldAccessFlag::INTERNAL;
            sf.flags |= (uint64_t)FieldAccessFlag::PROTECTED;
          break;
          case MONO_FIELD_ATTR_ASSEMBLY:
            sf.flags &= ~(uint64_t)FieldAccessFlag::PUBLIC;
            sf.flags &= ~(uint64_t)FieldAccessFlag::PROTECTED;
            sf.flags &= ~(uint64_t)FieldAccessFlag::PRIVATE;
            sf.flags |= (uint64_t)FieldAccessFlag::INTERNAL;
          break;
          case MONO_FIELD_ATTR_PRIVATE:
            sf.flags &= ~(uint64_t)FieldAccessFlag::PUBLIC;
            sf.flags &= ~(uint64_t)FieldAccessFlag::PROTECTED;
            sf.flags &= ~(uint64_t)FieldAccessFlag::INTERNAL;
            sf.flags |= (uint64_t)FieldAccessFlag::PRIVATE;
          break;
          default:
            break;
        }

        int32_t align;
        if (sf.IsArray()) {
          MonoClass* field_arr_class = mono_type_get_class(mtype);
          MonoClass* elt_class = mono_class_get_element_class(field_arr_class);
          MonoType* elt_type = mono_class_get_type(elt_class);
          sf.size = mono_type_size(elt_type , &align);
        } else {
          sf.size = mono_type_size(mtype , &align);
        }

        data.size += sf.size;
        if (std::find(data.fields.begin() , data.fields.end() , sf.hash) == data.fields.end()) {
          data.fields.push_back(sf.hash);
        }

      } while (field != nullptr);

      curr_class = mono_class_get_parent(curr_class);
    }
  }

  void CsScriptCache::StoreClassProperties(Cache& cache , TypeData& data) {
    if (data.asm_class == nullptr) {
      return;
    }

    MonoProperty* prop = nullptr;
    PropertyHandle iter = nullptr;

    do {
      prop = mono_class_get_properties(data.asm_class , &iter);
      if (prop == nullptr) {
        break;
      }

      std::string name{ mono_property_get_name(prop) };

      std::string hash_str = data.name + ":" + name;
      UUID id = FNV(hash_str);

      uint64_t prop_flags = 0;
      MonoType* mtype = nullptr;

      MonoMethod* getter = mono_property_get_get_method(prop);
      if (getter != nullptr) {
        MonoMethodSignature* sig = mono_method_signature(getter);

        mtype = mono_signature_get_return_type(sig);

        uint32_t flags = mono_method_get_flags(getter , nullptr);
        uint32_t visibility = flags & MONO_METHOD_ATTR_ACCESS_MASK;

        switch (visibility) {
          case MONO_FIELD_ATTR_PUBLIC:
            prop_flags &= ~(uint64_t)FieldAccessFlag::PROTECTED;
            prop_flags &= ~(uint64_t)FieldAccessFlag::PRIVATE;
            prop_flags &= ~(uint64_t)FieldAccessFlag::INTERNAL;
            prop_flags |= (uint64_t)FieldAccessFlag::PUBLIC;
          break;
          case MONO_FIELD_ATTR_FAMILY:
            prop_flags &= ~(uint64_t)FieldAccessFlag::PUBLIC;
            prop_flags &= ~(uint64_t)FieldAccessFlag::PRIVATE;
            prop_flags &= ~(uint64_t)FieldAccessFlag::INTERNAL;
            prop_flags |= (uint64_t)FieldAccessFlag::PROTECTED;
          break;
          case MONO_FIELD_ATTR_ASSEMBLY:
            prop_flags &= ~(uint64_t)FieldAccessFlag::PUBLIC;
            prop_flags &= ~(uint64_t)FieldAccessFlag::PROTECTED;
            prop_flags &= ~(uint64_t)FieldAccessFlag::PRIVATE;
            prop_flags |= (uint64_t)FieldAccessFlag::INTERNAL;
          break;
          case MONO_FIELD_ATTR_PRIVATE:
            prop_flags &= ~(uint64_t)FieldAccessFlag::PUBLIC;
            prop_flags &= ~(uint64_t)FieldAccessFlag::PROTECTED;
            prop_flags &= ~(uint64_t)FieldAccessFlag::INTERNAL;
            prop_flags |= (uint64_t)FieldAccessFlag::PRIVATE;
          break;
          default:
            break;
        }

        if ((flags & MONO_METHOD_ATTR_STATIC) != 0) {
          prop_flags |= static_cast<uint64_t>(FieldAccessFlag::STATIC);
        }
      }

      MonoMethod* setter = mono_property_get_set_method(prop);
      if (setter != nullptr) {
        ParamHandle iter = nullptr;
        MonoMethodSignature* sig = mono_method_signature(setter);
        mtype = mono_signature_get_params(sig , &iter);

        uint32_t flags = mono_method_get_flags(setter , nullptr);
        if ((flags & MONO_METHOD_ATTR_ACCESS_MASK) == MONO_METHOD_ATTR_PRIVATE) {
          prop_flags |= static_cast<uint64_t>(FieldAccessFlag::PRIVATE);
        }

        if ((flags & MONO_METHOD_ATTR_STATIC) != 0) {
          prop_flags |= static_cast<uint64_t>(FieldAccessFlag::STATIC);
        }
      }

      if (getter == nullptr && setter == nullptr) {
        prop_flags |= static_cast<uint64_t>(FieldAccessFlag::READONLY);
      }

      if (mtype == nullptr) {
        OE_ERROR("Failed to retrieve type information for property : {}::{}" , data.name , name);
        continue;
      }

      FieldData& sf = cache.field_data[id];
      sf.name = name;
      sf.hash = id;
      sf.vtype = ValueTypeFromMonoType(mtype);
      sf.is_property = true;

      int32_t align;
      sf.size = mono_type_size(mtype , &align);
      data.size += sf.size;

      if (std::find(data.fields.begin() , data.fields.end() , sf.hash) == data.fields.end()) {
        data.fields.push_back(sf.hash);
      }

    } while (prop != nullptr);
  }

} // namespace other
