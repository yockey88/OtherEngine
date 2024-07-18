/**
 * \file scripting/lua/lua_bindings.hpp
 **/
#ifndef OTHER_ENGINE_LUA_BINDINGS_HPP
#define OTHER_ENGINE_LUA_BINDINGS_HPP

#include <sol/sol.hpp>
#include <sol/state.hpp>

namespace other {
namespace lua_script_bindings {

  void BindCoreTypes(sol::state& lua);

} // namespace lua_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_LUA_BINDINGS_HPP
