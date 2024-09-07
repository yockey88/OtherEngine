/**
 * \file scripting/lua/lua_component_bindings.hpp
 **/
#ifndef OTHER_ENGINE_LUA_COMPONENT_BINDINGS_HPP
#define OTHER_ENGINE_LUA_COMPONENT_BINDINGS_HPP

#include <sol/state.hpp>

namespace other {
namespace lua_script_bindings {

  void BindEcsTypes(sol::state& lua_state);

} // namespace lua_script_bindings  
} // namespace other

#endif // !OTHER_ENGINE_LUA_COMPONENT_BINDINGS_HPP
