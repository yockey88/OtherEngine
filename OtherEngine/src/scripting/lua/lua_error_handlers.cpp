/**
 * \file scripting/lua/lua_error_handlers.cpp
 **/
#include "scripting/lua/lua_error_handlers.hpp"

#include <sstream>

#include "core/logger.hpp"

namespace other {

  int32_t LuaExceptionHandler(lua_State* L, sol::optional<const std::exception&> exception, sol::string_view description) {
    std::stringstream ss;
    ss << "Handling Lua Exception : " << description;
    OE_ERROR(ss.str());

    /// maybe we can use this push to return lua to a safe state ??
    return sol::stack::push(L, description);
  }

}  // namespace other
