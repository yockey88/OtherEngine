/**
 * \file scripting/lua/lua_ui_bindings.hpp
 **/
#ifndef OTHER_ENGINE_LUA_UI_BINDINGS_HPP
#define OTHER_ENGINE_LUA_UI_BINDINGS_HPP

#include <sol/state.hpp>

namespace other {
namespace lua_script_bindings {

  void BindUiTypes(sol::state& lua_state);

} // namespace other
} // namespace other

#endif // !OTHER_ENGINE_LUA_UI_BINDINGS_HPP
