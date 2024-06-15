/**
 * \file scripting/cs/mono_utils.hpp
 **/
#ifndef OTHER_ENGINE_MONO_UTILS_HPP
#define OTHER_ENGINE_MONO_UTILS_HPP

#include <mono/metadata/object.h>

#include "core/defines.hpp"
#include "core/value.hpp"

namespace other {

  enum FieldAccessFlag : int64_t {
    NO_ACCESS_CONTROL = -1 ,
    READONLY = bit(0) ,
    STATIC = bit(1) , 
    PUBLIC  = bit(2) ,
    PRIVATE = bit(3) , 
    PROTECTED = bit(4) ,
    INTERNAL = bit(5) ,
    ARRAY = bit(6)
  };

  bool CheckMonoError();  

  ValueType ValueTypeFromMonoType(MonoType* type);

  Opt<Value> MonoObjectToValue(MonoObject* object);

  template <typename T>
  T UnsafeUnbox(MonoObject* object) {
    return *static_cast<T*>(mono_object_unbox(object));
  }

} // namespace other

#endif // !OTHER_ENGINE_MONO_UTILS_HPP
