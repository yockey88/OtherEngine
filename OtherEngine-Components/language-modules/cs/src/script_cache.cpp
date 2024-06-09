/**
 * \file script_cache.cpp
 **/
#include "script_cache.hpp"

#include <iterator>
#include <map>

#include <mono/metadata/loader.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

#include "core/defines.hpp"

namespace other {

  struct Cache {
    std::map<UUID , TypeData> type_data;
    std::map<UUID , MethodData> method_data;
  };

  static Cache* cache = nullptr;

  bool CsScriptCache::initialized = false;

  void CsScriptCache::InitializeCache() {
    if (initialized) {
      return;
    } 

    cache = new Cache; 
    
    initialized = true;
  }
      
  void CsScriptCache::ShutdownCache() {
    if (!initialized) {
      return;
    }

    initialized = true;
  }
      
  void CsScriptCache::CacheClass(const std::string_view name , MonoClass* klass) {
    TypeData data{};
    data.name = name;

    std::string case_ins;
    std::transform(name.begin() , name.end() , std::back_inserter(case_ins) , ::toupper);

    data.id = FNV(case_ins);

    uint32_t align = 0;
    data.size = mono_class_value_size(klass , &align);
    data.asm_class = klass;

    TypeData& td = cache->type_data[data.id] = data;

    if (name.find("Other.") != std::string::npos) {
      StoreClassMethods(td);
      // StoreClassFields
      // StoreClassProperties
    }
  }

  void CsScriptCache::StoreClassMethods(TypeData& data) {
    if (data.asm_class == nullptr) {
      return;
    }

    MonoMethod* method = nullptr;
    void* iter = nullptr;

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

      auto& md = cache->method_data[method_data.hash] = method_data;

    } while (method != nullptr);
  }

  void CsScriptCache::StoreClassFields(TypeData& data) {
    if (data.asm_class == nullptr) {
      return;
    }
  }

  void CsScriptCache::StoreClassProperties(TypeData& data) {
    if (data.asm_class == nullptr) {
      return;
    }
  }

} // namespace other
