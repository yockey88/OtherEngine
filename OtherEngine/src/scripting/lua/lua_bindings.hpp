/**
 * \file scripting/lua/lua_bindings.hpp
 **/
#ifndef OTHER_ENGINE_LUA_BINDINGS_HPP
#define OTHER_ENGINE_LUA_BINDINGS_HPP

#include <sol/state.hpp>

namespace other {
namespace lua_script_bindings {

  void BindCoreTypes(sol::state& lua_state);

  void BindAll(sol::state& lua_state);

} // namespace lua_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_LUA_BINDINGS_HPP
