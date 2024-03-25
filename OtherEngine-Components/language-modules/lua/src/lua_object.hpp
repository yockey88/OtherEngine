/**
 * \file lua_object.hpp
 **/
#ifndef LUA_OBJECT_HPP
#define LUA_OBJECT_HPP

#include "scripting/script_object.hpp"

extern "C" {
  #include <lua/lua.h>
}

namespace other {

    class LuaObject : public ScriptObject {
      public:
        LuaObject()
          : ScriptObject("[ Empty Lua Object ]") {}
        LuaObject(const std::string& name , const std::string& file);
        virtual ~LuaObject() override;

        virtual void InitializeScriptMethods() override;
        virtual Opt<Value> CallMethod(const std::string& name , ParamHandle* args) override;

        virtual void Initialize() override;
        virtual void Update(float dt) override;
        virtual void Render() override;
        virtual void RenderUI() override;
        virtual void Shutdown() override;

      private:
        lua_State* L = nullptr;

        std::string file;
    };

} // namespace other

#endif // !LUA_OBJECT_HPP
