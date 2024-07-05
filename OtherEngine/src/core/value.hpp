/**
 * \file core/value.hpp
 **/
#ifndef OTHER_ENGINE_VALUE_HPP
#define OTHER_ENGINE_VALUE_HPP

#include <vector>

#include <glm/glm.hpp>

#include "core/defines.hpp"
#include "core/buffer.hpp"
#include "asset/asset_types.hpp"

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

      Value(const std::string& str) {
        SetStr(str);
      }

      Value(Value&& other);
      Value(const Value& other);
      Value& operator=(Value&& other);
      Value& operator=(const Value& other);

      ~Value();

      template <typename T>
      T Get() const {
        T ret = value.Read<T>();
        return ret;
      }

      template <>
      std::string Get<std::string>() const {
        std::string res;
        res.resize(value.Size());
        for (uint32_t i = 0; i < value.Size(); ++i) {
          res[i] = value[i];
        }

        return res; 
      }

      template <typename T>
      const T& Read() const {
        return value.Read<T>();
      }

      void* AsRawMemory() const;

      void Clear();

      void SetStr(const std::string& str);

      template <typename T>
      void Set(const T& val) {
        value.Release();
        value.Allocate(sizeof(T));
        value.Write(&val , sizeof(T));

        type = GetValueType<T>();
      }

      template <typename T>
      void Set(const std::vector<T>& values) {
        value.Release();
        value.Allocate(sizeof(T) * values.size());
        for (uint32_t i = 0; i < values.size(); ++i) {
          value.Write(values[i] , sizeof(T) , sizeof(T) * i); 
        } 

        type = GetValueType<T>();

        is_array = true;
      }

      ValueType Type() const;
      bool IsArray() const;
      size_t Size() const;

    private:
      bool is_array = false;

      ValueType type = ValueType::EMPTY;
      Buffer value;
  };

} // namespace other

#endif // !OTHER_ENGINE_VALUE_HPP
