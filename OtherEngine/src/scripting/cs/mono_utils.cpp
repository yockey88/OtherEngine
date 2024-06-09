/**
 * \file scripting/cs/mono_utils.cpp
 **/
#include "scripting/cs/mono_utils.hpp"

#include "core/logger.hpp"

#include <mono/metadata/object.h>
#include <mono/utils/mono-error.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/class.h>

namespace other {

  bool CheckMonoError() {
    MonoError error;

    bool has_err = !mono_error_ok(&error);

    if (!has_err) {
      /// false = no error
      return false;
    }
    
    unsigned int code = mono_error_get_error_code(&error);
    std::string error_msg = mono_error_get_message(&error);

    OE_ERROR("Error thrown by mono : [{} | {}]" , code , error_msg);
    return true;
  }

  ValueType ValueTypeFromMonoType(MonoType *type) {
    int32_t type_encoding = mono_type_get_type(type);
    MonoClass* type_class = mono_type_get_class(type);

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

        /// if asset handle 
        /// if vec2
        /// if vec3
        /// if vec4
      }
      case MONO_TYPE_CLASS: {
        /// defined classes
      }
      case MONO_TYPE_SZARRAY:
      case MONO_TYPE_ARRAY: {
        MonoClass* elt_class = mono_class_get_element_class(type_class);
        if (elt_class == nullptr) {
          break;
        }

      }

      default: 
        OE_ERROR("Failed to retrieve mono type");
        return ValueType::EMPTY;
    }

    return ValueType::EMPTY;
  }
  
  Opt<Value> MonoObjectToValue(MonoObject* object) {
    if (object == nullptr) {
      return std::nullopt;
    } 

    Value val;

    MonoType* mtype = mono_class_get_type(mono_object_get_class(object));
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

      default:
        return std::nullopt;
    }

    return val;
  }

} // namespace other
