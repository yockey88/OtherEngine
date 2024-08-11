/**
 * \file core/value.cpp
 **/
#include "core/value.hpp"

namespace other {

  Value::Value(Value&& other) {
    type = other.type;
    value = other.value;
    other.value.Release();
  }

  Value::Value(const Value& other) {
    type = other.type;
    value = other.value;
  }

  Value& Value::operator=(Value&& other) {
    type = other.type;
    value = other.value;
    other.value.Release();
    return *this;
  }

  Value& Value::operator=(const Value& other) {
    type = other.type;
    value = other.value;
    return *this;
  }
      
  Value::~Value() {
    value.Release();
  }
  
  void* Value::AsRawMemory() const {
    return static_cast<void*>(const_cast<uint8_t*>(value.ReadBytes()));
  }
      
  void Value::Clear() {
    value.Release();
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
      
  size_t Value::NumElements() const {
    return value.NumElements();
  }

} // namespace other
