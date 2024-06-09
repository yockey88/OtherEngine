/**
 * \file lua_object.hpp
 **/
#ifndef LUA_OBJECT_HPP
#define LUA_OBJECT_HPP

#include "scripting/script_object.hpp"

#include <sol/sol.hpp>

namespace other {

    class LuaObject : public ScriptObject {
      public:
        LuaObject()
          : ScriptObject("[ Empty Lua Object ]" , "Lua") {}
        LuaObject(const std::string& name , sol::state* script_state);
        virtual ~LuaObject() override;

        virtual void InitializeScriptMethods() override;
        virtual void CallMethod(Value* ret , const std::string& name , ParamHandle* args) override;
        virtual void GetProperty(Value* ret , const std::string& name) const override;
        virtual void SetProperty(const Value& value) override;

        virtual void Initialize() override;
        virtual void Update(float dt) override;
        virtual void Render() override;
        virtual void RenderUI() override;
        virtual void Shutdown() override;

      private:
        sol::state* state = nullptr;

        sol::table object;

        sol::function initialize;
        sol::function update;
        sol::function render;
        sol::function render_ui;
        sol::function shutdown;
    };

} // namespace other

#endif // !LUA_OBJECT_HPP
