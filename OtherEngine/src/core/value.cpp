/**
 * \file core/value.cpp
 **/
#include "core/value.hpp"

namespace other {

  Value::Value(Value&& other) {
    type = other.type;
    value = Buffer::Copy(other.value);
    other.value.Release();
  }

  Value::Value(const Value& other) {
    type = other.type;
    value = Buffer::Copy(other.value);
  }

  Value& Value::operator=(Value&& other) {
    type = other.type;
    value = Buffer::Copy(other.value);
    other.value.Release();
    return *this;
  }

  Value& Value::operator=(const Value& other) {
    type = other.type;
    value = Buffer::Copy(other.value);
    return *this;
  }
      
  Value::~Value() {
    value.Release();
  }
  
  Value::Value(ParamHandle val , size_t typesize , ValueType type) : type(type) {
    value.Allocate(typesize);
    value.Write(val , typesize);
  }
      
  Value::Value(ParamArray array , size_t typesize , size_t elements , ValueType type) 
      : type(type) {
    if (elements <= 0) {
      return;
    }

    is_array = true;

    value.Allocate(typesize * elements);
    for (uint32_t i = 0; i < elements; ++i) {
      value.Write(array[i] , typesize , i * typesize);
    }
  }
      
  void Value::Clear() {
    value.Release();
  }
      
  void Value::Set(ParamHandle val , size_t typesize , ValueType t) {
    value.Release();
    value.Allocate(typesize);
    value.Write(val , typesize);

    type = t;
  }

  void Value::Set(ParamArray array , size_t typesize , size_t elements , ValueType t) {
    if (elements <= 0) {
      return;
    }

    is_array = true;

    value.Release();
    value.Allocate(typesize * elements);
    for (uint32_t i = 0; i < elements; ++i) {
      value.Write(array[i] , typesize , i * typesize);
    }

    type = t;
  }

  ValueType Value::Type() const {
    return type;
  }
      
  bool Value::IsArray() const {
    return is_array;
  }

  size_t Value::Size() const {
    return value.Size();
  }

} // namespace other
