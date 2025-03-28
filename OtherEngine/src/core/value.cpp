/**
 * \file core/value.cpp
 **/
#include "core/value.hpp"

namespace other {

  Value::Value() {
  }

  Value::~Value() {
    storage = nullptr;
  }

  Value Value::CreateOpaqueHandle(void* opaque_data) {
    Value v = Value();
    v.storage = NewRef<ValueStorageImpl<void*>>();
    v.storage->data = opaque_data;
    return v;
  }

  Value::Value(Value&& other) {
    storage = other.storage;
    other.storage = nullptr;
  }

  Value::Value(const Value& other) {
    storage = other.storage;
  }

  Value& Value::operator=(Value&& other) {
    storage = other.storage;
    other.storage = nullptr;
    return *this;
  }

  Value& Value::operator=(const Value& other) {
    storage = other.storage;
    return *this;
  }

  bool Value::IsEmpty() const {
    if (storage == nullptr) {
      return false;
    }

    bool empty = storage->data == nullptr;
    if (empty) {
      OE_ASSERT(storage->size == 0, "Size is not zero for empty value!");
      OE_ASSERT(storage->value_type == ValueType::EMPTY_TYPE, "Value type is not EMPTY_TYPE for empty value!");
    }
    return empty;
  }

  void Value::Clear() {
    if (IsEmpty() || storage->value_type == ValueType::OPAQUE_HANDLE) {
      return;
    }
    OE_ASSERT(storage != nullptr, "Storage is null!");
    storage = nullptr;
  }

  size_t Value::GetSize() const {
    if (storage == nullptr) {
      return 0;
    }
    return storage->size;
  }

  ValueType Value::GetType() const {
    if (storage == nullptr) {
      return ValueType::EMPTY_TYPE;
    }
    return storage->value_type;
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
