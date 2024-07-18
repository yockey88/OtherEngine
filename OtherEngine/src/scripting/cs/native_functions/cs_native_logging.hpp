/**
 * \file scripting/cs/native_functions/cs_native_logging.hpp
 **/
#ifndef OTHER_ENGINE_CS_NATIVE_LOGGING_HPP
#define OTHER_ENGINE_CS_NATIVE_LOGGING_HPP

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>

namespace other {
namespace cs_script_bindings {

  void WriteLine(MonoString* string);

  void WriteTrace(MonoString* string);

  void WriteDebug(MonoString* string);
  
  void WriteInfo(MonoString* string);

  void WriteWarning(MonoString* string);

  void WriteError(MonoString* string);

  void WriteCritical(MonoString* string);

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_LOGGING_HPP
