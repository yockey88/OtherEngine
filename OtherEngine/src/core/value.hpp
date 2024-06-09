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
            
    BOOL , CHAR ,
    INT8 , INT16 , INT32 , INT64 ,
    UINT8 , UINT16 , UINT32 , UINT64 ,
    FLOAT , DOUBLE ,
    STRING ,

    VEC2 , VEC3 , VEC4 , 
    ASSET , 
    ENTITY 
  };

  struct Parameter {
    ParamHandle handle;
    ValueType type;
  };

  template <typename T>
  ValueType GetValueType(const T& val) {
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

      Value(ParamHandle value , size_t typesize , ValueType type);

      Value(ParamArray array , size_t typesize , size_t elements , ValueType type);

      Value(Value&& other);
      Value(const Value& other);
      Value& operator=(Value&& other);
      Value& operator=(const Value& other);

      ~Value();

      template <typename T>
      const T& Get() const {
        T ret = value.Read<T>();
        return ret;
      }

      template <typename T>
      const T& Read() const {
        return value.Read<T>();
      }

      void Clear();

      void Set(ParamHandle value , size_t typesize , ValueType type);

      void Set(ParamArray , size_t typesize , size_t elements , ValueType type);

      template <typename T>
      void Set(const T& val) {
        value.Release();
        value.Allocate(sizeof(T));
        value.Write(&val , sizeof(T));

        type = GetValueType<T>(val);
      }

      template <typename T>
      void Set(const std::vector<T>& values) {
        value.Release();
        value.Allocate(sizeof(T) * values.size());
        for (uint32_t i = 0; i < values.size(); ++i) {
          value.Write(values[i] , sizeof(T) , sizeof(T) * i); 
        } 

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
