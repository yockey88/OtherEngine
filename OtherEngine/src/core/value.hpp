/**
 * \file core/value.hpp
 **/
#ifndef OTHER_ENGINE_VALUE_HPP
#define OTHER_ENGINE_VALUE_HPP

#include <mutex>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <reflection/echo_defines.hpp>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/ref.hpp"

#include "memory/arena_allocator.hpp"

namespace other {

  class ValueStorage : public RefCounted {
   public:
    ValueStorage() = default;
    virtual ~ValueStorage() = default;

   protected:
    friend class Value;

    void* data = nullptr;
    size_t size = 0;
    ValueType value_type = ValueType::EMPTY_TYPE;
  };

  template <typename T>
  class ValueStorageImpl : public ValueStorage {
   public:
    ValueStorageImpl(const T& value) {
      data = allocator.Allocate(value);
      value_type = GetValueType<T>();

      if constexpr (std::same_as<T, std::string> || std::same_as<T, std::string_view>) {
        size = value.size();
      } else {
        size = sizeof(T);
      }
    }

    ValueStorageImpl(T* value_ptr) {
      OE_ASSERT(value_ptr != nullptr, "Value pointer is null!");
      data = allocator.Allocate(*value_ptr);
      *(T*)data = *value_ptr;
      value_type = GetValueType<T>();

      if constexpr (std::same_as<T, std::string> || std::same_as<T, std::string_view>) {
        size = value_ptr->size();
      } else {
        size = sizeof(T);
      }
    }

    ValueStorageImpl() {
      static_assert(std::same_as<T, void*>, "ValueStorageImpl must be specialized for non-void types to use the default constructor!");
      size = sizeof(void*);
      value_type = ValueType::OPAQUE_HANDLE;
    }

    ~ValueStorageImpl() {
      if (value_type != ValueType::OPAQUE_HANDLE) {
        allocator.Free(data);
      }
      data = nullptr;
      size = 0;
      value_type = ValueType::EMPTY_TYPE;
    }

    ValueStorageImpl(ValueStorageImpl&& other) = delete;
    ValueStorageImpl(const ValueStorageImpl& other) {
      data = other.data;
      size = other.size;
      value_type = other.value_type;
    }
    ValueStorageImpl& operator=(ValueStorageImpl&& other) = delete;
    ValueStorageImpl& operator=(const ValueStorageImpl& other) {
      data = other.data;
      size = other.size;
      value_type = other.value_type;
      return *this;
    }

   private:
    ArenaAllocator<T> allocator;
  };

  class Value {
   public:
    Value();

    template <typename T>
    Value(const T& value) {
      storage = NewRef<ValueStorageImpl<T>>(value);
    }

    template <typename T>
    Value(T* value_ptr) {
      storage = NewRef<ValueStorageImpl<T>>(value_ptr);
    }

    template <typename T>
    Value& operator=(const T& other) {
      storage = NewRef<ValueStorageImpl<T>>(other);
      return *this;
    }
    template <typename T>
    Value& operator=(T* other) {
      storage = NewRef<ValueStorageImpl<T>>(other);
      return *this;
    }

    ~Value();

    Value(Value&& other);
    Value(const Value& other);
    Value& operator=(Value&& other);
    Value& operator=(const Value& other);

    static Value CreateOpaqueHandle(void* opaque_data);

    bool IsEmpty() const;
    void Clear();

    template <typename T>
    T& Get() {
      OE_ASSERT(storage != nullptr, "Storage is null!");
      OE_ASSERT(storage->value_type != ValueType::EMPTY_TYPE, "Value is empty!");
      OE_ASSERT(storage->value_type != ValueType::OPAQUE_HANDLE, "Value is opaque!");
      OE_ASSERT(storage->value_type == GetValueType<T>(), "Value type mismatch!");
      OE_ASSERT(storage->data != nullptr, "Data is null!");
      T* t_ptr = std::launder(static_cast<T*>(storage->data));
      return *t_ptr;
    }

    template <typename T>
    const T& Get() const {
      OE_ASSERT(storage != nullptr, "Storage is null!");
      OE_ASSERT(storage->value_type != ValueType::EMPTY_TYPE, "Value is empty!");
      OE_ASSERT(storage->value_type != ValueType::OPAQUE_HANDLE, "Value is opaque!");
      OE_ASSERT(storage->value_type == GetValueType<T>(), "Value type mismatch!");
      OE_ASSERT(storage->data != nullptr, "Data is null!");
      const T* t_ptr = std::launder(static_cast<T*>(storage->data));
      return *t_ptr;
    }

    template <typename T>
    T* GetPtr() {
      OE_ASSERT(storage != nullptr, "Storage is null!");
      if (storage->value_type == ValueType::OPAQUE_HANDLE) {
        return std::launder(static_cast<T*>(storage->data));
      }

      OE_ASSERT(storage->value_type == GetValueType<T>(), "Value type mismatch!");
      OE_ASSERT(storage->data != nullptr, "Data is null!");
      return std::launder(static_cast<T*>(storage->data));
    }

    template <typename T>
    const T* GetPtr() const {
      OE_ASSERT(storage != nullptr, "Storage is null!");
      if (storage->value_type == ValueType::OPAQUE_HANDLE) {
        return std::launder(static_cast<T*>(storage->data));
      }

      OE_ASSERT(storage->value_type == GetValueType<T>(), "Value type mismatch!");
      OE_ASSERT(storage->data != nullptr, "Data is null!");
      return std::launder(static_cast<T*>(storage->data));
    }

    template <typename T>
    operator T&() { return Get<T>(); }
    template <typename T>
    operator const T&() const { return Get<T>(); }

    template <typename T>
    T& operator*() { return *this; }
    template <typename T>
    const T& operator*() const { return *this; }

    size_t GetSize() const;
    ValueType GetType() const;

    static uint32_t NumElements(ValueType type);

   private:
    friend struct ValueReference;
    friend class Registers;

    std::mutex mutex;

    Ref<ValueStorage> storage;
  };

}  // namespace other

ECHO_TYPE(
  type(other::Value)
);

#endif  // !OTHER_ENGINE_VALUE_HPP
