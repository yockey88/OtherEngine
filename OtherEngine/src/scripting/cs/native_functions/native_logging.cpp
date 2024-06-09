/**
 * \file scripting/cs/native_functions/native_logging.cpp
 **/
#include "scripting/cs/native_functions/native_logging.hpp"

#include <string_view>

#include <mono/metadata/object.h>

#include "core/defines.hpp"

namespace other {
namespace script_bindings {

  static void WriteMonoString(const std::string_view fmt , MonoString* mstr) {
    char* str = mono_string_to_utf8(mstr);
    println(fmt , str);
    mono_free(str);
  }

  void WriteLine(MonoString* mstr) {
    WriteMonoString("[ C# ]> {}" , mstr);
  } 
  
  void WriteTrace(MonoString* mstr) {
    WriteMonoString("[ C# trace ]> {}" , mstr);
  }

  void WriteDebug(MonoString* mstr) {
    WriteMonoString("[ C# Debug ]> {}" , mstr);
  }

  void WriteWarning(MonoString* mstr) {
    WriteMonoString("[ C# Warn ]> {}" , mstr);
  }

  void WriteError(MonoString* mstr) {
    WriteMonoString("[ C# Error ]> {}" , mstr);
  }

  void WriteCritical(MonoString* mstr) {
    WriteMonoString("[ C# Fatal ]> {}" , mstr);
  }

} // namespace script_bindings
} // namespace other
