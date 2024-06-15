/**
 * \file scripting/lua/lua_error_handlers.cpp
 **/
#include "scripting/lua/lua_error_handlers.hpp"

#include "core/logger.hpp"

namespace other {

   
  int32_t LuaExceptionHandler(lua_State* L , sol::optional<const std::exception&> exception , 
                              sol::string_view description) {
    OE_ERROR("Handling Lua Exception");

    if (exception) {
      OE_ERROR(" > Caught Lua exception : {}" , (*exception).what());
    } else {
      OE_ERROR(" > Caught Lua exception : {}" , description);
    }

    /// maybe we can use this push to return lua to a safe state ??
    return sol::stack::push(L , description);
  }

} // namespace other
