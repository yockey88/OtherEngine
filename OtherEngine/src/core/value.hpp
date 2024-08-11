/**
 * \file core/value.hpp
 **/
#ifndef OTHER_ENGINE_VALUE_HPP
#define OTHER_ENGINE_VALUE_HPP

#include <vector>

#include <glm/glm.hpp>

#include "core/defines.hpp"
#include "core/buffer.hpp"

namespace other {
  
  typedef void* ParamHandle;
  typedef ParamHandle* ParamArray;

  typedef void* FlagHandle;
  typedef void* FieldHandle;
  typedef void* MethodHandle;
  typedef void* PropertyHandle;
  
  struct Parameter {
    ParamHandle handle;
    ValueType type;
  };

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
      T Get() const {
        OE_ASSERT(!is_array , "Cannot use .Get<{}> on value when it is an array!" , typeid(T).name());
        T ret = value.Read<T>();
        return ret;
      }

      template <>
      std::string Get<std::string>() const {
        OE_ASSERT(!is_array , "Cannot use .Get<std::string> on value when it is an array!");
        std::string res;
        res.resize(value.Size());
        for (uint32_t i = 0; i < value.Size(); ++i) {
          res[i] = value[i];
        }

        return res; 
      }

      template <typename T>
      const T& Read() const {
        OE_ASSERT(!is_array , "Cannot use .Read<{}> on value when it is an array!" , typeid(T).name());
        return value.Read<T>();
      }

      template <typename T>
      T& At(size_t index) {
        OE_ASSERT(is_array , "Can not index into value with .At<{}> if it is not an array!" , typeid(T).name());
        return value.At<T>(index);
      }
      
      template <typename T>
      const T& At(size_t index) const {
        OE_ASSERT(is_array , "Can not index into value with .At<{}> if it is not an array!" , typeid(T).name());
        return At<T>(index);
      }

      void* AsRawMemory() const;

      void Clear();

      template <typename T>
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

      ValueType Type() const;
      bool IsArray() const;
      size_t Size() const;
      size_t NumElements() const;

    private:
      bool is_array = false;

      ValueType type = ValueType::EMPTY;
      Buffer value;
  };

} // namespace other

#endif // !OTHER_ENGINE_VALUE_HPP
