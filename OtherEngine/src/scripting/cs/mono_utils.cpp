/**
 * \file scripting/cs/mono_utils.cpp
 **/
#include "scripting/cs/mono_utils.hpp"

#include "core/logger.hpp"

#include <glm/common.hpp>
#include <mono/metadata/reflection.h>
#include <mono/utils/mono-error.h>
#include <mono/metadata/object.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/class.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/appdomain.h>

#include "math/math.hpp"

#include "scripting/cs/cs_script_cache.hpp"

namespace other {

  bool CheckMonoError(MonoError* error) {
    bool has_error = !mono_error_ok(error);
    if (has_error) {
      unsigned short err_code = mono_error_get_error_code(error);
      std::string err_msg{ mono_error_get_message(error) };
      OE_ERROR("Mono error: {0} | {1}" , err_code , err_msg);
      mono_error_cleanup(error);
    }
    return has_error;
  }

  bool CheckMonoError() {
    MonoError error;

    bool has_err = !mono_error_ok(&error);

    if (!has_err) {
      return false;
    }
    
    unsigned int code = mono_error_get_error_code(&error);
    std::string error_msg = mono_error_get_message(&error);

    OE_ERROR("Error thrown by mono : [{} | {}]" , code , error_msg);
    return true;
  }
  
  MonoClass* MonoClassFromValueType(ValueType type) {
#define GET_TYPE(name) mono_class_from_name(mono_get_corlib() , "System" , name);
    switch (type) {
      case ValueType::BOOL: return GET_TYPE("Boolean");
      case ValueType::INT8: 
      case ValueType::CHAR: return GET_TYPE("SByte"); 
      case ValueType::UINT8: return GET_TYPE("Byte");
      case ValueType::INT16: return GET_TYPE("Int16");
      case ValueType::UINT16: return GET_TYPE("Uint16");
      case ValueType::INT32: return GET_TYPE("Int32");
      case ValueType::UINT32: return GET_TYPE("UInt32");
      case ValueType::INT64: return GET_TYPE("Int64");
      case ValueType::UINT64: return GET_TYPE("UInt64");
      case ValueType::FLOAT: return GET_TYPE("Single");
      case ValueType::DOUBLE: return GET_TYPE("Double");
      case ValueType::STRING: return GET_TYPE("String");

      case ValueType::EMPTY:
      default:
        return nullptr;
    }
#undef GET_TYPE
  }
  
  MonoType* MonoTypeFromValueType(ValueType type) {
#define GET_TYPE(name) mono_class_get_type(mono_class_from_name(mono_get_corlib() , "System" , name));
    switch (type) {
      case ValueType::BOOL: return GET_TYPE("Boolean");
      case ValueType::INT8: 
      case ValueType::CHAR: return GET_TYPE("SByte"); 
      case ValueType::UINT8: return GET_TYPE("Byte");
      case ValueType::INT16: return GET_TYPE("Int16");
      case ValueType::UINT16: return GET_TYPE("Uint16");
      case ValueType::INT32: return GET_TYPE("Int32");
      case ValueType::UINT32: return GET_TYPE("UInt32");
      case ValueType::INT64: return GET_TYPE("Int64");
      case ValueType::UINT64: return GET_TYPE("UInt64");
      case ValueType::FLOAT: return GET_TYPE("Single");
      case ValueType::DOUBLE: return GET_TYPE("Double");
      case ValueType::STRING: return GET_TYPE("String");

      case ValueType::EMPTY:
      default:
        return nullptr;
    }
#undef GET_TYPE
  }

  ValueType ValueTypeFromMonoType(MonoType *type) {
    int32_t type_encoding = mono_type_get_type(type);
    MonoClass* type_class = mono_type_get_class(type);

    std::string name = mono_type_get_name(type);

    switch (type_encoding) {
      case MONO_TYPE_VOID: return ValueType::EMPTY;
      case MONO_TYPE_BOOLEAN: return ValueType::BOOL;
      case MONO_TYPE_CHAR: return ValueType::CHAR;
      case MONO_TYPE_I1: return ValueType::INT8; 
      case MONO_TYPE_I2: return ValueType::INT16;
      case MONO_TYPE_I4: return ValueType::INT32;
      case MONO_TYPE_I8: return ValueType::INT64;
      case MONO_TYPE_U1: return ValueType::UINT8;
      case MONO_TYPE_U2: return ValueType::UINT16;
      case MONO_TYPE_U4: return ValueType::UINT32;
      case MONO_TYPE_U8: return ValueType::UINT64;
      case MONO_TYPE_R4: return ValueType::FLOAT;
      case MONO_TYPE_R8: return ValueType::DOUBLE;
      case MONO_TYPE_STRING: return ValueType::STRING;
      case MONO_TYPE_VALUETYPE: {
        if (mono_class_is_enum(type_class)) {
          return ValueTypeFromMonoType(mono_type_get_underlying_type(type));
        }
        
        char* name = mono_type_get_name(type);
        std::string typestr{ name };
        typestr = typestr.substr(typestr.find('.') + 1 , typestr.length() - 1);

        if (typestr == "Vec2") {
          return ValueType::VEC2;
        } else if (typestr == "Vec3") {
          return ValueType::VEC3;
        } else if (typestr == "Vec4") {
          return ValueType::VEC4;
        } else {
          return ValueType::USER_TYPE;
        }

      } break;
      
      /// need to be able to check if asset handle and also 
      ///  handle custom types
      case MONO_TYPE_CLASS: {
        char* name = mono_type_get_name(type);
        std::string typestr{ name };
        typestr = typestr.substr(typestr.find('.') + 1 , typestr.length() - 1);

        OE_DEBUG("MONO_TYPE_CLASS : {}" , typestr);
      } break;

      case MONO_TYPE_SZARRAY:
      case MONO_TYPE_ARRAY: {
        MonoClass* elt_class = mono_class_get_element_class(type_class);
        if (elt_class == nullptr) {
          break;
        }

        MonoType* elt_type = mono_class_get_type(elt_class);
        return ValueTypeFromMonoType(elt_type);

      } break;

      default: 
        return ValueType::EMPTY;
    }

    return ValueType::EMPTY;
  }
  
  Opt<Value> MonoObjectToValue(MonoObject* object , MonoClassField* field , bool is_array) {
    if (object == nullptr) {
      return std::nullopt;
    } 

    /// TODO: fix array handling (should we even expose arrays to the editor/runtime?) 
    if (is_array) {
      return std::nullopt;
    //   return MonoObjectToValueArray(object , field);
    }

    Value val;

    MonoClass* mclass = mono_object_get_class(object);
    MonoType* mtype = mono_class_get_type(mclass);
    ValueType type = ValueTypeFromMonoType(mtype);
 
    if (type == ValueType::EMPTY) {
      return std::nullopt;
    }

    switch (type) {
      case ValueType::BOOL: {
        bool b = UnsafeUnbox<bool>(object);
        val.Set(b);
      } break;
      case ValueType::CHAR: {
        char c = UnsafeUnbox<char>(object);
        val.Set(c);
      } break;
      case ValueType::INT8: {
        int8_t i = UnsafeUnbox<int8_t>(object);
        val.Set(i);
      } break;
      case ValueType::INT16: {
        int16_t i = UnsafeUnbox<int16_t>(object);
        val.Set(i);
      } break;
      case ValueType::INT32: {
        int32_t i = UnsafeUnbox<int32_t>(object);
        val.Set(i);
      } break;
      case ValueType::INT64: {
        int64_t i = UnsafeUnbox<int64_t>(object);
        val.Set(i);
      } break;
      case ValueType::UINT8: {
        uint8_t  u = UnsafeUnbox<uint8_t>(object);
        val.Set(u);
      } break;
      case ValueType::UINT16: {
        uint16_t  u = UnsafeUnbox<uint16_t>(object);
        val.Set(u);
      } break;
      case ValueType::UINT32: {
        uint32_t  u = UnsafeUnbox<uint32_t>(object);
        val.Set(u);
      } break;
      case ValueType::UINT64: {
        uint64_t  u = UnsafeUnbox<uint64_t>(object);
        val.Set(u);
      } break;
      case ValueType::FLOAT: {
        float  f = UnsafeUnbox<float>(object);
        val.Set(f);
      } break;
      case ValueType::DOUBLE: {
        double  d = UnsafeUnbox<double>(object);
        val.Set(d);
      } break;
      case ValueType::STRING: {
        MonoString* mstr = reinterpret_cast<MonoString*>(object);
        if (mono_string_length(mstr) == 0) {
          return Value{};
        }

        MonoError error{};
        char* utf8 = mono_string_to_utf8_checked(mstr , &error);
        if (CheckMonoError(&error)) {
          return Value{};
        }

        std::string res{ utf8 };
        mono_free(utf8);

        val.Set(res);
      } break;

      case ValueType::VEC2: {
        glm::vec2 v = UnsafeUnbox<glm::vec2>(object);
        val.Set(v);
      } break;
      
      case ValueType::VEC3: {
        glm::vec3 v = UnsafeUnbox<glm::vec3>(object);
        val.Set(v);
      } break;
      
      case ValueType::VEC4: {
        glm::vec4 v = UnsafeUnbox<glm::vec4>(object);
        val.Set(v);
      } break;

      case ValueType::USER_TYPE: 
        
      break;

      default:
        return std::nullopt;
    }

    return val;
  }
  
  Opt<Value> MonoObjectToValueArray(MonoObject* object , MonoClassField* field) {
    MonoType* field_type = mono_field_get_type(field);
    MonoClass* field_class = mono_type_get_class(field_type);
    MonoClass* field_elt_class = mono_class_get_element_class(field_class);
    MonoType* elt_type = mono_class_get_type(field_elt_class);

    int32_t align;
    size_t size = mono_type_size(elt_type , &align);

    MonoArray* array = (MonoArray*)object; 
    size_t len = mono_array_length(array);
    OE_DEBUG("Array of length {}" , len);

    for (size_t i = 0; i < len; ++i) {
      float val = 0;
      void* data = &val;
      if (mono_type_is_reference(elt_type) || mono_type_is_byref(elt_type)) {
        MonoObject* mval = mono_array_get(array , MonoObject* , i);
        if (mval == nullptr) {
          OE_ERROR("Failed to get elt index {}" , i);
          continue;
        }
        val = UnsafeUnbox<float>(mval);
      } else {
        char* src = mono_array_addr_with_size(array , (int)size , i); 
        memcpy(data , src , size);
      }
    }

    return std::nullopt;
  }
    
  constexpr static UUID kBoundsHash = FNV("BoundsAttribute");
  constexpr static UUID kMinPropHash = FNV("Min");
  constexpr static UUID kMaxPropHash = FNV("Max");
  
  bool HasBoundsAttribute(const CsFieldData& fdata) {
    for (const auto& [hash , attr] : fdata.attributes) {
      if (hash == kBoundsHash) {
        return true;
      } 
    }

    return false;
  }

  Opt<glm::vec2> GetMonoBoundsAttribute(const CsFieldData &fdata , CsCache* asm_data) {
    if (fdata.attr_info == nullptr) {
      return std::nullopt;
    }
    
    MonoObject* attrs = fdata.attributes.at(kBoundsHash).attribute_instance;

    auto min_itr = asm_data->field_data.find(kMinPropHash);
    auto max_itr = asm_data->field_data.find(kMaxPropHash);

    OE_ASSERT(min_itr != asm_data->field_data.end() , "Failed to load C# core class field Other::BoundsAttribute.Min");
    OE_ASSERT(max_itr != asm_data->field_data.end() , "Failed to load C# core class field Other::BoundsAttribute.Max");

    auto* min_getter = min_itr->second.asm_property_getter;
    auto* max_getter = max_itr->second.asm_property_getter;

    MonoObject* exception = nullptr;
    MonoObject* min_val = mono_runtime_invoke(min_getter , attrs , nullptr , &exception);
    if (exception != nullptr) {
      OE_ERROR("Failed to clamp value, C# exception raised");
      return std::nullopt;
    }

    MonoObject* max_val = mono_runtime_invoke(max_getter , attrs , nullptr , &exception);
    if (exception != nullptr) {
      OE_ERROR("Failed to clamp value, C# exception raised");
      return std::nullopt;
    }
    
    double real_min = UnsafeUnbox<double>(min_val);
    double real_max = UnsafeUnbox<double>(max_val);

    return glm::vec2{ real_min , real_max };
  }

  Value ClampValue(const CsFieldData& fdata , CsCache* asm_data , const Value& value) {

    Opt<glm::vec2> bounds = GetMonoBoundsAttribute(fdata , asm_data);
    if (!bounds.has_value()) {
      return value;
    }

    Value result;

    double real_min = bounds->x;
    double real_max = bounds->y;

    switch (value.Type()) {
      case ValueType::INT8: {
        int8_t val = value.Get<int8_t>();
        val = Clamp<int8_t>(val , real_min , real_max);
        result.Set(val);
      } break; 
      case ValueType::UINT8: {
        uint8_t val = value.Get<uint8_t>();
        val = Clamp<uint8_t>(val , real_min , real_max);
        result.Set(val);
      } break;
      case ValueType::INT16: {
        int16_t val = value.Get<int16_t>();
        val = Clamp<int16_t>(val , real_min , real_max);
        result.Set(val);
      } break;
      case ValueType::UINT16: {
        uint16_t val = value.Get<uint16_t>();
        val = Clamp<uint16_t>(val , real_min , real_max);
        result.Set(val);
      } break;
      case ValueType::INT32: {
        int32_t val = value.Get<int32_t>();
        val = Clamp<int32_t>(val , real_min , real_max);
        result.Set(val);
      } break;
      case ValueType::UINT32: {
        uint32_t val = value.Get<uint32_t>();
        val = Clamp<uint32_t>(val , real_min , real_max);
        result.Set(val);
      } break;
      case ValueType::INT64: {
        int64_t val = value.Get<int64_t>();
        val = Clamp<int64_t>(val , real_min , real_max);
        result.Set(val);
      } break;
      case ValueType::UINT64: {
        uint64_t val = value.Get<uint64_t>();
        val = Clamp<int8_t>(val , real_min , real_max);
        result.Set(val);
      } break;  
      case ValueType::FLOAT: {
        float val = value.Get<float>();
        val = Clamp<float>(val , real_min , real_max);
        result.Set(val);
      } break;
      case ValueType::DOUBLE: {
        double val = value.Get<double>();
        val = Clamp<double>(val , real_min , real_max);
        result.Set(val);
      } break;

      case ValueType::VEC2: {
        glm::vec2 val = value.Get<glm::vec2>();
        val.x = Clamp<float>(val.x , real_min , real_max);
        val.y = Clamp<float>(val.y , real_min , real_max);
        result.Set(val);
      } break;
      case ValueType::VEC3: {
        glm::vec3 val = value.Get<glm::vec3>();
        val.x = Clamp<float>(val.x , real_min , real_max);
        val.y = Clamp<float>(val.y , real_min , real_max);
        val.z = Clamp<float>(val.z , real_min , real_max);
        result.Set(val);
      } break;
      case ValueType::VEC4: {
        glm::vec4 val = value.Get<glm::vec4>();
        val.x = Clamp<float>(val.x , real_min , real_max);
        val.y = Clamp<float>(val.y , real_min , real_max);
        val.z = Clamp<float>(val.z , real_min , real_max);
        val.w = Clamp<float>(val.w , real_min , real_max);
        result.Set(val);
      } break;

      default:
        break;
    }

    return result;
  }

} // namespace other
