/**
 * \file scripting/lua/lua_object.cpp
 **/
#include "scripting/lua/lua_object.hpp"

namespace other {

    void LuaObject::InitializeScriptMethods() {
      sol::optional<sol::table> obj = (*state)[script_name];
      if (!obj.has_value()) {
        is_corrupt = true;
        return;
      }
    }

    void LuaObject::InitializeScriptFields() {
    }
          
    Opt<Value> LuaObject::GetField(const std::string& name) {
      return std::nullopt;
    }

    void LuaObject::SetField(const std::string& name , const Value& value) {

    }

    void LuaObject::OnSetEntityId() {
      sol::optional<sol::function> set_entity_id = (*state)[script_name]["SetEntityId"];
      if (!set_entity_id.has_value()) {
        OE_WARN("Lua script {} does not have way to set entity id!" , script_name);
        return;
      }

      if (!set_entity_id.value().valid()) {
        OE_WARN("Lua script {} does not have way to set entity id!" , script_name);
        return;
      }

      (*set_entity_id)(entity_id);
    }

} // namespace other
