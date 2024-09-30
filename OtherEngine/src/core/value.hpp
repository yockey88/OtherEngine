/**
 * \file core/value.hpp
 **/
#ifndef OTHER_ENGINE_VALUE_HPP
#define OTHER_ENGINE_VALUE_HPP

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

#include <reflection/echo_defines.hpp>
#include <span>
#include <type_traits>

#include "core/defines.hpp"
#include "core/buffer.hpp"

namespace other {

  class Value {
    public:
      Value() {}

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
        requires std::is_trivially_copyable_v<T>
      T Get() const {
        OE_ASSERT(!is_array , "Cannot use .Get<{}> on value when it is an array!" , typeid(T).name());
        T ret = value.Read<T>();
        return ret;
      }

      template <typename T>
        requires std::same_as<T , std::string>
      std::string Get() const {
        OE_ASSERT(!is_array , "Cannot use .Get<std::string> on value when it is an array!");
        std::string res;
        res.resize(value.Size());
        for (uint32_t i = 0; i < value.Size(); ++i) {
          res[i] = value.At<char>(i);
        }

        return res; 
      }

      template <typename T>
        requires std::is_trivially_copyable_v<T>
      const T& Read() const {
        OE_ASSERT(!is_array , "Cannot use .Read<{}> on value when it is an array!" , typeid(T).name());
        return value.Read<T>();
      }

      template <typename T>
        requires std::is_trivially_copyable_v<T>
      T& At(size_t index) {
        OE_ASSERT(is_array , "Can not index into value with .At<{}> if it is not an array!" , typeid(T).name());
        return value.At<T>(index);
      }
      
      template <typename T>
        requires std::is_trivially_copyable_v<T>
      const T& At(size_t index) const {
        OE_ASSERT(is_array , "Can not index into value with .At<{}> if it is not an array!" , typeid(T).name());
        return At<T>(index);
      }

      void* AsRawMemory() const;

      void Clear();

      template <typename T>
        requires std::is_trivially_copyable_v<T>
      void Set(const T& val) {
        value.Write<T>(val);
        type = GetValueType<T>();
      }

      template <typename T>
      void Set(const std::span<T>& values) {
        value.WriteArr<T>(values);
        type = GetValueType<T>();

        is_array = true;
      }

      template <typename T>
        requires std::same_as<T , std::string>
      void Set(const std::string& val) {
        std::vector<char> string_chars{ val.begin() , val.end() };
        Set<char>(std::span{ string_chars });
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
      bool is_array = false;

      ValueType type = ValueType::EMPTY;
      Buffer value;
  };

} // namespace other

ECHO_TYPE(
  type(other::Value)
);

#endif // !OTHER_ENGINE_VALUE_HPP
