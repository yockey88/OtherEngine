/**
 * \file core/value.hpp
 **/
#ifndef OTHER_ENGINE_VALUE_HPP
#define OTHER_ENGINE_VALUE_HPP

#include <span>
#include <type_traits>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

#include <reflection/echo_defines.hpp>

#include "core/buffer.hpp"
#include "core/defines.hpp"

namespace other {

  class Value {
   public:
    Value() = default;

    template <typename T>
    Value(T val) {
      Set<T>(val);
    }

    Value(Value&& other);
    Value(const Value& other);
    Value& operator=(Value&& other);
    Value& operator=(const Value& other);

    ~Value();

    template <typename T>
    T Get() {
      OE_ASSERT(!is_array, "Cannot use .Get<{}> on value when it is an array!", typeid(T).name());
      return value.Read<T>();
    }

    template <typename T>
      requires std::is_trivially_copyable_v<T>
    const T& Get() const {
      OE_ASSERT(!is_array, "Cannot use .Read<{}> on value when it is an array!", typeid(T).name());
      return value.Read<T>();
    }

    template <typename T>
      requires std::is_trivially_copyable_v<T>
    T& At(size_t index = 0) {
      OE_ASSERT(is_array, "Can not index into value with .At<{}> if it is not an array!", typeid(T).name());
      return value.At<T>(index);
    }

    template <typename T>
      requires std::is_trivially_copyable_v<T>
    const T& At(size_t index) const {
      OE_ASSERT(is_array, "Can not index into value with .At<{}> if it is not an array!", typeid(T).name());
      return At<T>(index);
    }

    void* AsRawMemory() const;

    template <typename T>
    T* AddressAs() {
      if (GetValueType<T>() != type) {
        OE_ERROR("Attempting to retrieve value as incorrect type! Expected {} but got {}", GetValueType<T>(), type);
        return nullptr;
      }

      return value.As<T>();
    }

    bool Empty() const;
    void Clear();

    template <typename T>
      requires std::is_trivially_copyable_v<T>
    void Set(T&& val) {
      value.Write<T>(std::forward<T>(val));
      type = GetValueType<T>();
      is_array = false;
    }

    template <typename T>
      requires std::is_trivially_copyable_v<T>
    void Set(const T& val) {
      value.Write<T>(val);
      type = GetValueType<T>();
      is_array = false;
    }

    template <typename T>
    void Set(const std::span<T>& values) {
      value.WriteArr<T>(values);
      type = GetValueType<T>();
      is_array = true;
    }

    template <typename T>
      requires std::same_as<T, std::string>
    void Set(const std::string& val) {
      std::vector<char> string_chars{ val.begin(), val.end() };
      Set<char>(std::span{ string_chars });
      type = ValueType::STRING;
      is_array = false;
    }

    ValueType Type() const;
    bool IsArray() const;
    size_t Size() const;
    size_t NumElements() const;

    template <typename T>
    /// because setters are limited to this we limit the types that can be unwrapped as well
      requires std::is_trivially_copyable_v<T>
    static T Unwrap(const Value& val) {
      return val.Get<T>();
    }

   private:
    Buffer value;
    ValueType type = ValueType::EMPTY_TYPE;
    bool is_array = false;
  };

}  // namespace other

ECHO_TYPE(
  type(other::Value)
);

#endif  // !OTHER_ENGINE_VALUE_HPP
