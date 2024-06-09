/**
 * \file lua_object.cpp
 **/
#include "lua_object.hpp"

namespace other {

    LuaObject::LuaObject(const std::string& name , sol::state* script_state) 
      : ScriptObject(name , "Lua") , state(script_state) {
    }

    LuaObject::~LuaObject() {
    }

    void LuaObject::InitializeScriptMethods() {
      sol::optional<sol::table> obj = (*state)[script_name];
      if (!obj.has_value()) {
        is_corrupt = true;
        return;
      }

      initialize = (*state)[script_name]["OnInitialize"];
      update = (*state)[script_name]["OnUpdate"];
      render = (*state)[script_name]["OnRender"];
      render_ui = (*state)[script_name]["OnRenderUI"];
      shutdown = (*state)[script_name]["OnShutdown"];
    }

    void LuaObject::CallMethod(Value* ret , const std::string& name , ParamHandle* args) {
      sol::function func = (*state)[script_name][name];
      if (func.valid()) {
      }
    }

    void LuaObject::GetProperty(Value* val , const std::string& name) const {
    }
    
    void LuaObject::SetProperty(const Value& value) {
    }

    void LuaObject::Initialize() {
      if (initialize.valid()) {
        initialize.call();
      }
    }

    void LuaObject::Update(float dt) {
      if (update.valid()) {
        update.call(dt);
      }
    }

    void LuaObject::Render() {
      if (render.valid()) {
        render.call();
      }
    }

    void LuaObject::RenderUI() {
      if (render_ui.valid()) {
        render_ui.call();
      }
    }

    void LuaObject::Shutdown() {
      if (shutdown.valid()) {
        shutdown.call();
      }
    }

} // namespace other
