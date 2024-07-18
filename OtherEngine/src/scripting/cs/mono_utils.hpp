/**
 * \file scripting/cs/mono_utils.hpp
 **/
#ifndef OTHER_ENGINE_MONO_UTILS_HPP
#define OTHER_ENGINE_MONO_UTILS_HPP

#include <mono/metadata/object.h>

#include "core/defines.hpp"
#include "core/value.hpp"

namespace other {

  struct CsFieldData;
  struct CsCache;

  enum FieldAccessFlag : int64_t {
    NO_ACCESS_CONTROL = -1 ,
    READONLY = bit(0) ,
    STATIC_ACCESS = bit(1) , 
    PUBLIC  = bit(2) ,
    PRIVATE = bit(3) , 
    PROTECTED = bit(4) ,
    INTERNAL = bit(5) ,
    ARRAY = bit(6)
  };

  bool CheckMonoError();  

  MonoClass* MonoClassFromValueType(ValueType type);
  MonoType* MonoTypeFromValueType(ValueType type);

  ValueType ValueTypeFromMonoType(MonoType* type);

  Opt<Value> MonoObjectToValue(MonoObject* object , MonoClassField* field = nullptr , bool is_array = false);
  Opt<Value> MonoObjectToValueArray(MonoObject* object , MonoClassField* field = nullptr);

  bool HasBoundsAttribute(const CsFieldData& fdata);

  Opt<glm::vec2> GetMonoBoundsAttribute(const CsFieldData& fdata , CsCache* asm_data);

  Value ClampValue(const CsFieldData& fdata , CsCache* asm_data , const Value& value);

  template <typename T>
  T UnsafeUnbox(MonoObject* object) {
    return *static_cast<T*>(mono_object_unbox(object));
  }

} // namespace other

#endif // !OTHER_ENGINE_MONO_UTILS_HPP
