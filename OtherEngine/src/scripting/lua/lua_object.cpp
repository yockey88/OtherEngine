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

      initialize = (*state)[script_name]["OnInitialize"];
      shutdown = (*state)[script_name]["OnShutdown"];

      start = (*state)[script_name]["OnStart"];
      stop = (*state)[script_name]["OnStop"];

      update = (*state)[script_name]["Update"];
      render = (*state)[script_name]["Render"];
      render_ui = (*state)[script_name]["RenderUI"];

      if (!update.valid()) {
        OE_WARN("{} has no update method" , script_name);
      }
    }

    void LuaObject::InitializeScriptFields() {
    }

    Opt<Value> LuaObject::OnCallMethod(const std::string_view name , std::span<Value> args)  {
      return std::nullopt;
    }
      
    Opt<Value> LuaObject::GetField(const std::string& name) {
      return std::nullopt;
    }

    void LuaObject::SetField(const std::string& name , const Value& value) {

    }

    void LuaObject::Initialize() {
      if (initialize.valid()) {
        initialize.call();
      }

      is_initialized = true;
    }
    
    void LuaObject::Shutdown() {
      if (shutdown.valid()) {
        shutdown.call();
      }

      is_initialized = false;
    }

    void LuaObject::Start() {
      if (start.valid()) {
        start.call();
      }
    }

    void LuaObject::Stop() {
      if (stop.valid()) {
        stop.call();
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


} // namespace other
