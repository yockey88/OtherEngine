/**
 * \file scripting/cs/cs_register_internal_call.cpp
 **/
#include "scripting/cs/cs_register_internal_call.hpp"

#include "core/defines.hpp"
#include "core/logger.hpp"

namespace other {
namespace cs_script_bindings {

  void RegisterInternalCallAs(ref<Assembly> assembly , const std::string_view klass , const std::string_view name , void* method) {
    const std::string type_name = fmtstr("Other.{}" , klass);
    dotother::Type& type = assembly->GetType(type_name);
    if (type.handle == -1) {
      OE_ASSERT(false , "NO C# type found for {}" , type_name);
    }

    assembly->SetInternalCall(type_name , name , method);
  }

} // namespace cs_script_bindings
} // namespace other
