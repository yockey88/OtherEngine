/**
 * \file scripting/cs/cs_register_internal_call.hpp
 **/
#ifndef OTHER_ENGINE_CS_REGISTER_INTERNAL_CALL_HPP
#define OTHER_ENGINE_CS_REGISTER_INTERNAL_CALL_HPP

#include <core/dotother_defines.hpp>
#include <hosting/type.hpp>
#include <hosting/assembly.hpp>

#include "core/defines.hpp"
#include "core/logger.hpp"

using dotother::ref;
using dotother::Type;
using dotother::Assembly;

namespace other {
namespace cs_script_bindings {

  void RegisterInternalCallAs(ref<Assembly> assembly , const std::string_view klass , const std::string_view name , void* method);

  template <typename Fn>
  void RegisterFunction(const std::string_view klass_name , const std::string_view method_name , ref<Assembly> assembly , Fn&& fn) {
    const std::string name = fmtstr("Other.{}" , klass_name);
    Type& type = assembly->GetType(name);
    if (type.handle == -1) {
      OE_ASSERT(false , "NO C# type found for {}" , name);
    }

    static void* method = (void*)fn;

    assembly->SetInternalCall(name , method_name , (void*)method);
  }

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_REGISTER_INTERNAL_CALL_HPP