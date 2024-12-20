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

  bool Value::Empty() const {
    return Type() == ValueType::EMPTY_TYPE;
  }

  void Value::Clear() {
    value.Release();
    type = ValueType::EMPTY_TYPE;
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

  uint32_t Value::NumElements(ValueType type) {
    switch (type) {
      case ValueType::INT8:
      case ValueType::UINT8:
      case ValueType::INT16:
      case ValueType::UINT16:
      case ValueType::INT32:
      case ValueType::UINT32:
      case ValueType::INT64:
      case ValueType::UINT64:
      case ValueType::FLOAT:
      case ValueType::DOUBLE:
      case ValueType::ASSET:
      case ValueType::ENTITY:
      case ValueType::SAMPLER2D:
        return 1;

      case ValueType::VEC2:
        return 2;

      case ValueType::VEC3:
        return 3;

      case ValueType::VEC4:
        return 4;

      case ValueType::MAT2:
        return 2 * 2;

      case ValueType::MAT3:
        return 3 * 3;

      case ValueType::MAT4:
        return 4 * 4;

      case ValueType::EMPTY_TYPE:
      case ValueType::STRING:
        return 0;

      default:
        return 1;
    }
    OE_ASSERT(false, "Invalid value type");
    return 0;
  }

}  // namespace other
