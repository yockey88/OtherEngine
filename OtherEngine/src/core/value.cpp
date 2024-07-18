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
  
  void* Value::AsRawMemory() const {
    return static_cast<void*>(const_cast<uint8_t*>(value.ReadBytes()));
  }
      
  void Value::Clear() {
    value.Release();
  }
      
  void Value::SetStr(const std::string& str) {
    value.Release();
    if (str.length() == 0) {
      return;
    }
    value.WriteStr(str);
    type = ValueType::STRING;
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
