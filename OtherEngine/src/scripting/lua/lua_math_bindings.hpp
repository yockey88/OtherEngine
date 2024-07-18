/**
 * \file scripting/lua/lua_math_bindings.hpp
 **/
#ifndef OTHER_ENGINE_LUA_MATH_BINDINGS_HPP
#define OTHER_ENGINE_LUA_MATH_BINDINGS_HPP

#include <sol/sol.hpp>

namespace other {
namespace lua_script_bindings {

  void BindGlmTypes(sol::state& lua);

} // namespace lua_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_LUA_MATH_BINDINGS_HPP
