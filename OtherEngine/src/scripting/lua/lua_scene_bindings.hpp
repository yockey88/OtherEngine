/**
 * \file scripting/lua/lua_scene_bindings.hpp
 **/
#ifndef OTHER_ENGINE_LUA_SCENE_BINDINGS_HPP
#define OTHER_ENGINE_LUA_SCENE_BINDINGS_HPP

#include <sol/state.hpp>

namespace other {
namespace lua_script_bindings {

  void BindScene(sol::state& lua_state);

  void BindSceneManagerFunctions(sol::state& lua_state);

} // namespace lua_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_LUA_SCENE_BINDINGS_HPP
