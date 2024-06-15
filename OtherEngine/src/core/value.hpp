/**
 * \file core/value.hpp
 **/
#ifndef OTHER_ENGINE_VALUE_HPP
#define OTHER_ENGINE_VALUE_HPP

#include <vector>

#include <glm/glm.hpp>

#include "core/buffer.hpp"
#include "asset/asset_types.hpp"

namespace other {
  
  typedef void* ParamHandle;
  typedef ParamHandle* ParamArray;

  typedef void* FlagHandle;
  typedef void* FieldHandle;
  typedef void* MethodHandle;
  typedef void* PropertyHandle;
  
  enum ValueType {
    EMPTY , // Void , null , nil ,etc...
            
    /// primitive types
    BOOL , CHAR ,
    INT8 , INT16 , INT32 , INT64 ,
    UINT8 , UINT16 , UINT32 , UINT64 ,
    FLOAT , DOUBLE ,
    STRING ,

    /// engine types
    VEC2 , VEC3 , VEC4 , 
    ASSET , ENTITY ,

    /// user types
    USER_TYPE ,
  };

  struct Parameter {
    ParamHandle handle;
    ValueType type;
  };

  template <typename T>
  static constexpr ValueType GetValueType() {
    if constexpr (std::is_same_v<T , bool>) {
      return ValueType::BOOL;
    } else if constexpr (std::is_same_v<T , char>) {
      return ValueType::CHAR;
    } else if constexpr (std::is_same_v<T , int8_t>) {
      return ValueType::INT8;
    } else if constexpr (std::is_same_v<T, int16_t>) {
      return ValueType::INT16;
    } else if constexpr (std::is_same_v<T, int32_t>) {
      return ValueType::INT32;
    } else if constexpr (std::is_same_v<T, int64_t>) {
      return ValueType::INT64;
    } else if constexpr (std::is_same_v<T, uint8_t>) {
      return ValueType::UINT8;
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      return ValueType::UINT16;
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      return ValueType::UINT32;
    } else if constexpr (std::is_same_v<T, uint64_t>) {
      return ValueType::UINT64;
    } else if constexpr (std::is_same_v<T, float>) {
      return ValueType::FLOAT;
    } else if constexpr (std::is_same_v<T, double>) {
      return ValueType::DOUBLE;
    } else if constexpr (std::is_same_v<T , glm::vec2>) {
      return ValueType::VEC2;
    } else if constexpr (std::is_same_v<T , glm::vec3>) {
      return ValueType::VEC3;
    } else if constexpr (std::is_same_v<T , glm::vec4>) {
      return ValueType::VEC4;
    } else if constexpr (std::is_same_v<T , AssetHandle>) {
      return ValueType::ASSET;
    } else if constexpr (std::is_same_v<T , UUID>) {
      return ValueType::ENTITY;
    } else {
      return ValueType::EMPTY;
    }
  }

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
