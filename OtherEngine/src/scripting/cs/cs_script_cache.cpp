/**
 * \file scripting/cs/cs_script_core_cache.cpp
 **/
#include "scripting/cs/cs_script_cache.hpp"

#include <map>

#include <mono/metadata/blob.h>
#include <mono/metadata/class.h>
#include <mono/metadata/image.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/row-indexes.h>

#include "core/defines.hpp"
#include "core/value.hpp"
#include "scripting/cs/mono_utils.hpp"

namespace other {

  CsFieldData::~CsFieldData() {
    if (attr_info != nullptr) {
      mono_custom_attrs_free(attr_info);
    }
  }
 
  CsMethodData::~CsMethodData() {
    if (attr_info != nullptr) {
      mono_custom_attrs_free(attr_info);
    }
  }
  
  CsTypeData::~CsTypeData() {
    if (attr_info != nullptr) {
      mono_custom_attrs_free(attr_info);
    }
  }
      
  std::vector<ScriptField> CsCache::GetClassFields(const std::string_view class_name) {
    UUID id = FNV(class_name);
    if (class_data.find(id) == class_data.end()) {
      return {};
    }

    const auto& tdata = class_data[id];

    std::vector<ScriptField> fields;
    for (const auto& [id , f] : tdata.fields) {
      if (f.HasFlag(FieldAccessFlag::PRIVATE) || f.HasFlag(FieldAccessFlag::PROTECTED)) {
        continue;
      }

      ScriptField sfield;
      sfield.id = f.hash;
      sfield.name = f.name;

      fields.push_back(sfield);
    }

    return fields;
  }

  void CsCache::CacheClass(const std::string_view name , MonoClass* klass) {
    if (klass == nullptr) {
      OE_WARN("Attempting to cache null C# class {}" , name);
      return;
    } 
    
    CsTypeData data{};
    data.name = name;
    data.id = FNV(name);

    if (class_data.find(data.id) != class_data.end()) {
      return;
    }

    uint32_t align = 0;
    data.size = mono_class_value_size(klass , &align);
    data.asm_class = klass;

    CsTypeData& td = class_data[data.id] = data;
    td.name_space = std::string{ mono_class_get_namespace(data.asm_class) };

    td.attr_info = mono_custom_attrs_from_class(td.asm_class);

    OE_DEBUG("Caching C# class {}::{}" , td.name_space , td.name);

    StoreClassMethods(td);
    StoreClassFields(td);
    StoreClassProperties(td);

    if (td.attr_info == nullptr) {
      return;
    }

    MonoArray* attrs = mono_custom_attrs_construct(td.attr_info);
    size_t len = mono_array_length(attrs);
    if (len == 0) {
      return;
    }

    for (size_t i = 0; i < len; ++i) {
      MonoObject* attr_obj = mono_array_get(attrs , MonoObject* , i);
      if (attr_obj == nullptr) {
        continue;
      }

      MonoClass* attr_class = mono_object_get_class(attr_obj);
      if (attr_class == nullptr) {
        continue;
      }

      std::string attr_name = mono_class_get_name(attr_class);
      UUID attr_hash = FNV(attr_name);

      auto& ad = td.attributes[attr_hash] = CsAttributeData{};
      ad.attribute_instance = attr_obj;
      
      ad.hash = attr_hash;
      ad.name = attr_name;

      if (class_data.find(attr_hash) == class_data.end()) {
        CacheClass(attr_name , attr_class);
      }

      ad.attr_type_data = &class_data[attr_hash]; 
    }
  }
    
  CsTypeData* CsCache::GetClassData(UUID id) {
    if (class_data.find(id) == class_data.end()) {
      return nullptr;
    }

    return &class_data[id];
  }

  void CsCache::Clear() {
    for (auto& [id , c] : class_data) {
      c.fields.clear();
      c.methods.clear();
    }
    class_data.clear();
  }

  void CsCache::StoreClassMethods(CsTypeData& data) {
    if (data.asm_class == nullptr) {
      return;
    }

    MonoClass* curr_class = data.asm_class;
    while (curr_class != nullptr) {
      std::string class_name{ mono_class_get_name(curr_class) };
      std::string class_namespace{ mono_class_get_namespace(curr_class) }; 
    
      MonoMethod* method = nullptr;
      MethodHandle iter = nullptr;

      do {
        method = mono_class_get_methods(curr_class , &iter);
        if (method == nullptr) {
          break;
        }

        MonoMethodSignature* sig = mono_method_signature(method);

        std::string full_name = std::string{ mono_method_full_name(method , 0) };
        std::string name = std::string{ mono_method_get_name(method) };
        UUID hash = FNV(name);
        
        CsMethodData&  mdata = data.methods[hash] = CsMethodData{};
        mdata.name = name;
        mdata.full_name = full_name;
        mdata.hash = hash;

        mdata.flags = mono_method_get_flags(method , nullptr);

        mdata.is_static = mdata.flags & MONO_METHOD_ATTR_STATIC;
        mdata.is_virtual = mdata.flags & MONO_METHOD_ATTR_VIRTUAL;
        mdata.parameter_count = mono_signature_get_param_count(sig);
        mdata.asm_method = method;

        auto& md = data.methods[mdata.hash] = mdata;

        md.attr_info = mono_custom_attrs_from_method(md.asm_method);

        if (md.attr_info == nullptr) {
          method_data[mdata.hash] = mdata;
          continue;
        }

        MonoArray* attrs = mono_custom_attrs_construct(md.attr_info);
        size_t len = mono_array_length(attrs);
        if (len == 0) {
          return;
        }

        for (size_t i = 0; i < len; ++i) {
          MonoObject* attr_obj = mono_array_get(attrs , MonoObject* , i);
          if (attr_obj == nullptr) {
            continue;
          }

          MonoClass* attr_class = mono_object_get_class(attr_obj);
          if (attr_class == nullptr) {
            continue;
          }

          std::string attr_name = mono_class_get_name(attr_class);
          UUID attr_hash = FNV(attr_name);
          
          auto& ad = md.attributes[attr_hash] = CsAttributeData{};
          ad.attribute_instance = attr_obj;
          
          ad.hash = attr_hash;
          ad.name = attr_name;

          if (class_data.find(attr_hash) == class_data.end()) {
            CacheClass(attr_name , attr_class);
          }

          ad.attr_type_data = &class_data[attr_hash]; 
        }
        
        method_data[mdata.hash] = md;
      } while (method != nullptr);

      if (class_name == "OtherBehavior" || class_name == "OtherAttribute") {
        break;
      }

      curr_class = mono_class_get_parent(curr_class); 
    } 
  }

  void CsCache::StoreClassFields(CsTypeData& data) {
    if (data.asm_class == nullptr) {
      return;
    }

    std::string name = std::string{ mono_class_get_name(data.asm_class) };
    std::string name_space = std::string{ mono_class_get_namespace(data.asm_class) };

    MonoClass* curr_class = data.asm_class;
    while (curr_class != nullptr) {
      std::string class_name{ mono_class_get_name(curr_class) };
      std::string class_namespace{ mono_class_get_namespace(curr_class) }; 

      MonoClassField* field = nullptr;
      FieldHandle iter = nullptr;

      do {
        field = mono_class_get_fields(curr_class , &iter);
        if (field == nullptr) {
          break;
        }

        const char* raw_name = mono_field_get_name(field);
        std::string name = raw_name;

        if (name.find("k__BackingField") != std::string::npos) {
          continue;
        }

        MonoType* mtype = mono_field_get_type(field);
        ValueType ftype = ValueTypeFromMonoType(mtype);
        if (ftype == ValueType::EMPTY) {
          continue;
        }

        UUID id = FNV(name);

        int32_t type_encoding = mono_type_get_type(mtype);

        CsFieldData& sf = data.fields[id] = CsFieldData{};

        sf.name = name;
        sf.hash = id;
        sf.vtype = ftype;
        sf.is_property = false;
        sf.asm_field = field;

        if (type_encoding == MONO_TYPE_ARRAY || type_encoding == MONO_TYPE_SZARRAY) {
          sf.flags |= FieldAccessFlag::ARRAY;
        }

        uint32_t visibility = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;
        switch (visibility) {
          case MONO_FIELD_ATTR_PUBLIC:
            sf.flags &= ~FieldAccessFlag::PROTECTED;
            sf.flags &= ~FieldAccessFlag::PRIVATE;
            sf.flags &= ~FieldAccessFlag::INTERNAL;
            sf.flags |= FieldAccessFlag::PUBLIC;
          break;
          case MONO_FIELD_ATTR_FAMILY:
            sf.flags &= ~FieldAccessFlag::PUBLIC;
            sf.flags &= ~FieldAccessFlag::PRIVATE;
            sf.flags &= ~FieldAccessFlag::INTERNAL;
            sf.flags |= FieldAccessFlag::PROTECTED;
          break;
          case MONO_FIELD_ATTR_ASSEMBLY:
            sf.flags &= ~FieldAccessFlag::PUBLIC;
            sf.flags &= ~FieldAccessFlag::PROTECTED;
            sf.flags &= ~FieldAccessFlag::PRIVATE;
            sf.flags |= FieldAccessFlag::INTERNAL;
          break;
          case MONO_FIELD_ATTR_PRIVATE:
            sf.flags &= ~FieldAccessFlag::PUBLIC;
            sf.flags &= ~FieldAccessFlag::PROTECTED;
            sf.flags &= ~FieldAccessFlag::INTERNAL;
            sf.flags |= FieldAccessFlag::PRIVATE;
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
          OE_INFO("Found array : {}" , sf.name);

        } else {
          sf.size = mono_type_size(mtype , &align);
        }

        data.size += sf.size;
        
        sf.attr_info = mono_custom_attrs_from_field(curr_class , sf.asm_field);

        if (sf.attr_info == nullptr) {
          field_data[sf.hash] = sf;
          continue;
        }
        
        MonoArray* attrs = mono_custom_attrs_construct(sf.attr_info);
        size_t len = mono_array_length(attrs);
        if (len == 0) {
          return;
        }

        for (size_t i = 0; i < len; ++i) {
          MonoObject* attr_obj = mono_array_get(attrs , MonoObject* , i);
          if (attr_obj == nullptr) {
            continue;
          }

          MonoClass* attr_class = mono_object_get_class(attr_obj);
          if (attr_class == nullptr) {
            continue;
          }

          std::string attr_name = mono_class_get_name(attr_class);
          UUID attr_hash = FNV(attr_name);

          auto& ad = sf.attributes[attr_hash] = CsAttributeData{};

          ad.attribute_instance = attr_obj;
          
          ad.hash = attr_hash;
          ad.name = attr_name;

          if (class_data.find(attr_hash) == class_data.end()) {
            CacheClass(attr_name , attr_class);
          }

          ad.attr_type_data = &class_data[attr_hash]; 
        }

        field_data[sf.hash] = sf;
      } while (field != nullptr);
      
      if (class_name == "OtherBehavior" || class_name == "OtherAttribute") {
        break;
      }

      curr_class = mono_class_get_parent(curr_class);
    }
  }

  void CsCache::StoreClassProperties(CsTypeData& data) {
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

      std::string hash_str = name;
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
            prop_flags &= ~FieldAccessFlag::PROTECTED;
            prop_flags &= ~FieldAccessFlag::PRIVATE;
            prop_flags &= ~FieldAccessFlag::INTERNAL;
            prop_flags |= FieldAccessFlag::PUBLIC;
          break;
          case MONO_FIELD_ATTR_FAMILY:
            prop_flags &= ~FieldAccessFlag::PUBLIC;
            prop_flags &= ~FieldAccessFlag::PRIVATE;
            prop_flags &= ~FieldAccessFlag::INTERNAL;
            prop_flags |= FieldAccessFlag::PROTECTED;
          break;
          case MONO_FIELD_ATTR_ASSEMBLY:
            prop_flags &= ~FieldAccessFlag::PUBLIC;
            prop_flags &= ~FieldAccessFlag::PROTECTED;
            prop_flags &= ~FieldAccessFlag::PRIVATE;
            prop_flags |= FieldAccessFlag::INTERNAL;
          break;
          case MONO_FIELD_ATTR_PRIVATE:
            prop_flags &= ~FieldAccessFlag::PUBLIC;
            prop_flags &= ~FieldAccessFlag::PROTECTED;
            prop_flags &= ~FieldAccessFlag::INTERNAL;
            prop_flags |= FieldAccessFlag::PRIVATE;
          break;
          default:
            break;
        }

        if ((flags & MONO_METHOD_ATTR_STATIC) != 0) {
          prop_flags |= FieldAccessFlag::STATIC_ACCESS; 
        }
      }

      MonoMethod* setter = mono_property_get_set_method(prop);
      if (setter != nullptr) {
        ParamHandle iter = nullptr;
        MonoMethodSignature* sig = mono_method_signature(setter);
        mtype = mono_signature_get_params(sig , &iter);

        uint32_t flags = mono_method_get_flags(setter , nullptr);
        if ((flags & MONO_METHOD_ATTR_ACCESS_MASK) == MONO_METHOD_ATTR_PRIVATE) {
          prop_flags |= FieldAccessFlag::PRIVATE;
        }

        if ((flags & MONO_METHOD_ATTR_STATIC) != 0) {
          prop_flags |= FieldAccessFlag::STATIC_ACCESS;
        }
      }

      if (getter == nullptr && setter == nullptr) {
        prop_flags |= FieldAccessFlag::READONLY;
      }

      if (mtype == nullptr) {
        OE_ERROR("Failed to retrieve type information for property : {}::{}" , data.name , name);
        continue;
      }

      CsFieldData& sf = data.fields[id] = CsFieldData{};
      sf.name = name;
      sf.hash = id;
      sf.vtype = ValueTypeFromMonoType(mtype);
      sf.is_property = true;
      sf.flags = prop_flags;
      sf.asm_property = prop;
      sf.asm_property_getter = getter;
      sf.asm_property_setter = setter;

      int32_t align;
      sf.size = mono_type_size(mtype , &align);
      data.size += sf.size;

      sf.attr_info = mono_custom_attrs_from_property(data.asm_class , sf.asm_property);

      if (sf.attr_info == nullptr) {
        field_data[sf.hash] = sf;
        continue;
      }

      
      MonoArray* attrs = mono_custom_attrs_construct(sf.attr_info);
      size_t len = mono_array_length(attrs);
      if (len == 0) {
        return;
      }

      for (size_t i = 0; i < len; ++i) {
        MonoObject* attr_obj = mono_array_get(attrs , MonoObject* , i);
        if (attr_obj == nullptr) {
          continue;
        }

        MonoClass* attr_class = mono_object_get_class(attr_obj);
        if (attr_class == nullptr) {
          continue;
        }

        std::string attr_name = mono_class_get_name(attr_class);
        UUID attr_hash = FNV(attr_name);

        auto& ad = sf.attributes[attr_hash] = CsAttributeData{};
        ad.attribute_instance = attr_obj;
        
        ad.hash = attr_hash;
        ad.name = attr_name;

        if (class_data.find(attr_hash) == class_data.end()) {
          CacheClass(attr_name , attr_class);
        }

        ad.attr_type_data = &class_data[attr_hash]; 
      }
      

      field_data[sf.hash] = sf;
    } while (prop != nullptr);
  }

} // namespace other
