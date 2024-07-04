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

      early_update = (*state)[script_name]["EarlyUpdate"];
      update = (*state)[script_name]["Update"];
      late_update = (*state)[script_name]["LateUpdate"];

      render = (*state)[script_name]["Render"];
      render_ui = (*state)[script_name]["RenderUI"];
    }

    void LuaObject::InitializeScriptFields() {
    }
          
    Opt<Value> LuaObject::GetField(const std::string& name) {
      return std::nullopt;
    }

    void LuaObject::SetField(const std::string& name , const Value& value) {

    }

    void LuaObject::OnBehaviorLoad() {}

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
    
    void LuaObject::OnBehaviorUnload() {}

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
    
    void LuaObject::EarlyUpdate(float dt) {
      if (early_update.valid()) {
        early_update.call(dt);
      }
    }
    
    void LuaObject::Update(float dt) {
      if (update.valid()) {
        update.call(dt);
      }
    }

    void LuaObject::LateUpdate(float dt) {
      if (late_update.valid()) {
        late_update.call(dt);
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

    Opt<Value> LuaObject::OnCallMethod(const std::string_view name , std::span<Value> args)  {
      return std::nullopt;
    }

} // namespace other
