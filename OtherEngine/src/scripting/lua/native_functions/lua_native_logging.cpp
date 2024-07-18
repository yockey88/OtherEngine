/**
 * \file scripting/lua/native_functions/lua_native_logging.cpp
 **/
#include "scripting/lua/native_functions/lua_native_logging.hpp"

#include "core/defines.hpp"

namespace other {
namespace lua_script_bindings {


  void WriteTrace(const std::string_view string) {
    println("[ Lua Trace ]> {}" , string);
  }

  void WriteDebug(const std::string_view string) {
    println("[ Lua Debug ]> {}" , string);
  }
  
  void WriteInfo(const std::string_view string) {
    println("[ Lua Info ]> {}" , string);
  }

  void WriteWarning(const std::string_view string) {
    println("[ Lua Warning ]> {}" , string);
  }

  void WriteError(const std::string_view string) {
    println("[ Lua Error ]> {}" , string);
  }

  void WriteCritical(const std::string_view string) {
    println("[ Lua Critical ]> {}" , string);
  }

} // namespace lua_script_bindings
} // namespace other
