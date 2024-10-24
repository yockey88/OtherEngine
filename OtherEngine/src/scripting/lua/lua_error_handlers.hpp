/**
 * \file scripting/lua/lua_error_handlers.hpp
 **/
#ifndef OTHER_ENGINE_LUA_ERROR_HANDLERS_HPP
#define OTHER_ENGINE_LUA_ERROR_HANDLERS_HPP

#include <cstdint>

#include <sol/sol.hpp>

namespace other {

  int32_t LuaExceptionHandler(lua_State* L, sol::optional<const std::exception&> exception, sol::string_view description);

}  // namespace other

#endif  // !OTHER_ENGINE_LUA_ERROR_HANDLERS_HPP
