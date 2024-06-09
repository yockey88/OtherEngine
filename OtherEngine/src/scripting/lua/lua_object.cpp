/**
 * \file scripting/lua/lua_object.cpp
 **/
#include "scripting/lua/lua_object.hpp"

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

    void LuaObject::InitializeScriptFields() {
      
    }

    Opt<Value> LuaObject::CallMethod(const std::string& name , Parameter* args , uint32_t argc)  {
      return std::nullopt;
      // sol::optional<sol::function> func = (*state)[name];
      // if (!func.has_value()) {
      //   func = (*state)[script_name][name];

      //   if (!func.has_value()) {
      //     OE_DEBUG("Could not find symbol {} in script {}" , name , script_name);
      //     return std::nullopt;
      //   }
      // }
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
