/**
 * \file core/defines.cpp
 **/
#include "core/defines.hpp"

namespace other {

  ValueType StringToValueType(const std::string& str) {
    if (str == "bool") {
      return ValueType::BOOL;
    } else if (str == "char") {
      return ValueType::CHAR;
    } else if (str == "int8") {
      return ValueType::INT8;
    } else if (str == "int16") {
      return ValueType::INT16;
    } else if (str == "int32") {
      return ValueType::INT32;
    } else if (str == "int64") {
      return ValueType::INT64;
    } else if (str == "uint8") {
      return ValueType::UINT8;
    } else if (str == "uint16") {
      return ValueType::UINT16;
    } else if (str == "uint32") {
      return ValueType::UINT32;
    } else if (str == "uint64") {
      return ValueType::UINT64;
    } else if (str == "float") {
      return ValueType::FLOAT;
    } else if (str == "double") {
      return ValueType::DOUBLE;
    } else if (str == "vec2") {
      return ValueType::VEC2;
    } else if (str == "vec3") {
      return ValueType::VEC3;
    } else if (str == "vec4") {
      return ValueType::VEC4;
    } else if (str == "mat2") {
      return ValueType::MAT2;
    } else if (str == "mat3") {
      return ValueType::MAT3;
    } else if (str == "mat4") {
      return ValueType::MAT4;
    } else if (str == "sampler2D") {
      return ValueType::SAMPLER2D;
    } else if (str == "asset") {
      return ValueType::ASSET;
    } else if (str == "entity") {
      return ValueType::ENTITY;
    } else if (str == "user_type" || str == "user" || str == "user-type") {
      return ValueType::USER_TYPE;
    } else {
      return ValueType::EMPTY_TYPE;
    }
  }

  std::string_view ValueTypeToString(ValueType type) {
    switch (type) {
      case ValueType::BOOL:
        return "bool";
      case ValueType::CHAR:
        return "char";
      case ValueType::INT8:
        return "int8";
      case ValueType::INT16:
        return "int16";
      case ValueType::INT32:
        return "int32";
      case ValueType::INT64:
        return "int64";
      case ValueType::UINT8:
        return "uint8";
      case ValueType::UINT16:
        return "uint16";
      case ValueType::UINT32:
        return "uint32";
      case ValueType::UINT64:
        return "uint64";
      case ValueType::FLOAT:
        return "float";
      case ValueType::DOUBLE:
        return "double";
      case ValueType::VEC2:
        return "vec2";
      case ValueType::VEC3:
        return "vec3";
      case ValueType::VEC4:
        return "vec4";
      case ValueType::MAT2:
        return "mat2";
      case ValueType::MAT3:
        return "mat3";
      case ValueType::MAT4:
        return "mat4";
      case ValueType::SAMPLER2D:
        return "sampler2D";
      case ValueType::ASSET:
        return "asset";
      case ValueType::ENTITY:
        return "entity";
      case ValueType::USER_TYPE:
        return "user-type";
      default:
        return "empty";
    }
  }

}  // namespace other