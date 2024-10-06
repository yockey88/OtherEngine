/**
 * \file scripting/lua/lua_object.cpp
 **/
#include "scripting/lua/lua_object.hpp"

namespace other {

    void LuaObject::InitializeScriptMethods() {
      sol::optional<sol::table> obj = state[script_name];
      if (!obj.has_value()) {
        is_corrupt = true;
        return;
      }
    }

    void LuaObject::InitializeScriptFields() {
    }

} // namespace other
