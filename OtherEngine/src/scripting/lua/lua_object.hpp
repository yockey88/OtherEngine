/**
 * \file scripting/lua/lua_object.hpp
 **/
#ifndef LUA_OBJECT_HPP
#define LUA_OBJECT_HPP

#include <sol/sol.hpp>

#include "scripting/script_object.hpp"

namespace other {

  class LuaObject : public ScriptObject {
    public:
      LuaObject()
        : ScriptObject(LanguageModuleType::LUA_MODULE , "[ Empty Lua Object ]" , "Lua") {}
      LuaObject(const std::string& module_name , const std::string& name , sol::state* script_state) 
        : ScriptObject(LanguageModuleType::LUA_MODULE , module_name , name) , state(script_state) {}
      virtual ~LuaObject() override {}

      virtual void InitializeScriptMethods() override;
      virtual void InitializeScriptFields() override;
      virtual void UpdateNativeFields() override {}
      
      virtual Opt<Value> GetField(const std::string& name) override;
      virtual void SetField(const std::string& name , const Value& value) override;

      virtual void OnBehaviorLoad() override;
      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual void OnBehaviorUnload() override;

      virtual void Start() override;
      virtual void Stop() override;

      virtual void EarlyUpdate(float dt) override;
      virtual void Update(float dt) override;
      virtual void LateUpdate(float dt) override;

      virtual void Render() override;
      virtual void RenderUI() override;

    private:
      sol::state* state = nullptr;

      sol::table object;

      sol::function initialize;
      sol::function shutdown;

      sol::function start;
      sol::function stop;

      sol::function early_update;
      sol::function update;
      sol::function late_update;

      sol::function render;
      sol::function render_ui;
    
      template <typename... Args>
      Opt<Value> CallLuaMethod(const std::string_view name , Args&&... args) {
        sol::optional<sol::function> function = (*state)[name];
        if (!function.has_value()) {
          function = object[name];

          if (!function.has_value()) {
            OE_ERROR("Could not find function {} in Lua script {}" , name , script_name);
            return std::nullopt;
          }
        }

        auto unwrap_val = [](const Value& val) -> auto {
        };

        try {
          sol::function func = *function;
          Opt<Value> val = func((unwrap_val(args) , ...)); 
        } catch (const std::exception& e) {
          OE_ERROR("Lua error caught calling function {} in script {}" , name , script_name);
          return std::nullopt;
        }
      }
      
      void OnSetEntityId() override;
      virtual Opt<Value> OnCallMethod(const std::string_view name , std::span<Value> args) override;
  };

} // namespace other

#endif // !LUA_OBJECT_HPP
