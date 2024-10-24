/**
 * \file scripting/lua/sol2_imgui_bindings.hpp
 **/
#ifndef OTHER_ENGINE_SOL2_IMGUI_BINDINGS_HPP
#define OTHER_ENGINE_SOL2_IMGUI_BINDINGS_HPP

#include <sol/sol.hpp>

namespace other {
  namespace lua_script_bindings {

    void BindImGui(sol::state& lua_state);

  }  // namespace lua_script_bindings
}  // namespace other

#endif  // !OTHER_ENGINE_SOL2_IMGUI_BINDINGS_HPP