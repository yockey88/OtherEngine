/**
 * \file scripting/lua/lua_logging_bindings.hpp
 **/
#ifndef OTHER_ENGINE_LUA_LOGGING_BINDINGS_HPP
#define OTHER_ENGINE_LUA_LOGGING_BINDINGS_HPP

#include <string_view>

namespace other {
namespace lua_script_bindings {

  void WriteTrace(const std::string_view string);
  
  void WriteInfo(const std::string_view string);

  void WriteDebug(const std::string_view string);

  void WriteWarning(const std::string_view string);

  void WriteError(const std::string_view string);

  void WriteCritical(const std::string_view string);

} // namespace lua_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_LUA_LOGGING_BINDINGS_HPP
