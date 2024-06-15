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

    public:
      static void BindGlmTypes(sol::state& lua_state);
      static void BindEcsTypes(sol::state& lua_state);

      static void InitializeMouseEnums(sol::state& lua_state);
      static void InitializeKeyEnums(sol::state& lua_state);
  };  

} // namespace other

#endif // !OTHER_ENGINE_LUA_BINDINGS_HPP
