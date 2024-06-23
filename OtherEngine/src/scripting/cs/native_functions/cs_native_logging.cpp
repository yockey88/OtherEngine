/**
 * \file scripting/cs/native_functions/cs_native_logging.cpp
 **/
#include "scripting/cs/native_functions/cs_native_logging.hpp"

#include <string_view>

#include <mono/metadata/object.h>

#include "core/defines.hpp"
#include "core/logger.hpp"

namespace other {
namespace cs_script_bindings {

  static void WriteMonoString(Logger::Level lvl , const std::string_view fmt , MonoString* mstr) {
    char* str = mono_string_to_utf8(mstr);
    switch (lvl) {
      case Logger::Level::TRACE:
        OE_TRACE(fmt , str);
        break;
      case Logger::Level::DEBUG:
        OE_DEBUG(fmt , str);
        break;
      case Logger::Level::INFO:
        OE_INFO(fmt , str);
        break;
      case Logger::Level::WARN:
        OE_WARN(fmt , str);
        break;
      case Logger::Level::ERR:
        OE_ERROR(fmt , str);
        break;
      case Logger::Level::CRITICAL:
        OE_CRITICAL(fmt , str);
        break;
      default:
        OE_CRITICAL("[ Unkown Log Level ] | {}" , fmtstr(fmt , str));
    }

    mono_free(str);
  }

  void WriteLine(MonoString* mstr) {
    char* str = mono_string_to_utf8(mstr);
    println("[ C# ]> {}" , str);
    mono_free(str);
  } 
  
  void WriteTrace(MonoString* mstr) {
    WriteMonoString(Logger::TRACE , "[ C# trace ]> {}" , mstr);
  }

  void WriteDebug(MonoString* mstr) {
    WriteMonoString(Logger::DEBUG , "[ C# Debug ]> {}" , mstr);
  }
  
  void WriteInfo(MonoString* mstr) {
    WriteMonoString(Logger::INFO , "[ C# Info ]> {}" , mstr);
  }

  void WriteWarning(MonoString* mstr) {
    WriteMonoString(Logger::WARN , "[ C# Warn ]> {}" , mstr);
  }

  void WriteError(MonoString* mstr) {
    WriteMonoString(Logger::ERR , "[ C# Error ]> {}" , mstr);
  }

  void WriteCritical(MonoString* mstr) {
    WriteMonoString(Logger::CRITICAL , "[ C# Critical ]> {}" , mstr);
  }

} // namespace cs_script_bindings
} // namespace other
