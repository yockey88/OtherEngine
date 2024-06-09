/**
 * \file scripting/cs/native_functions/native_logging.hpp
 **/
#ifndef OTHER_ENGINE_NATIVE_LOGGING_HPP
#define OTHER_ENGINE_NATIVE_LOGGING_HPP

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>

namespace other {
namespace script_bindings {

  void WriteLine(MonoString* string);

  void WriteTrace(MonoString* string);

  void WriteDebug(MonoString* string);

  void WriteWarning(MonoString* string);

  void WriteError(MonoString* string);

  void WriteCritical(MonoString* string);

} // script bindings
} // namespace other

#endif // !OTHER_ENGINE_NATIVE_LOGGING_HPP
