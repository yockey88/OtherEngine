/**
 * \file scripting/lua/lua_bindings.hpp
 **/
#ifndef OTHER_ENGINE_LUA_BINDINGS_HPP
#define OTHER_ENGINE_LUA_BINDINGS_HPP

#include <sol/sol.hpp>

namespace other {

  class LuaScriptBindings {
    public:
      static void InitializeBindings(sol::state& lua_state);
  };  

} // namespace other

#endif // !OTHER_ENGINE_LUA_BINDINGS_HPP
