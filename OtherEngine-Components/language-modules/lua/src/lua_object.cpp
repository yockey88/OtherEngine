/**
 * \file lua_object.cpp
 **/
#include "lua_object.hpp"

extern "C" {
  #include <lua/lua.h>
  #include <lua/lualib.h>
  #include <lua/lauxlib.h>
}

namespace other {

    LuaObject::LuaObject(const std::string& name , const std::string& file) 
      : ScriptObject(name) , file(file) {
    }

    LuaObject::~LuaObject() {
    }

    void LuaObject::InitializeScriptMethods() {
    }

    Opt<Value> LuaObject::CallMethod(const std::string& name , ParamHandle* args) {
      if (L == nullptr) {
        return std::nullopt;
      }

      auto method = lua_getglobal(L , name.c_str());
      if (method == LUA_TFUNCTION) {
        lua_call(L , 0 , 0);
      }

      return std::nullopt;
    }

    void LuaObject::Initialize() {
      if (L != nullptr) {
        return;
      }

      L = luaL_newstate();
      luaL_openlibs(L);

      luaL_dofile(L , file.c_str());

      auto init = lua_getglobal(L , "Initialize");
      if (init == LUA_TFUNCTION) {
        lua_call(L , 0 , 0);
      }
    }

    void LuaObject::Update(float dt) {
      if (L == nullptr) {
        return;
      }

      auto update = lua_getglobal(L , "Update");
      if (update == LUA_TFUNCTION) {
        lua_pushnumber(L , dt);
        lua_call(L , 1 , 0);
      }
    }

    void LuaObject::Render() {
      if (L == nullptr) {
        return;
      }

      auto render = lua_getglobal(L , "Render");
      if (render == LUA_TFUNCTION) {
        lua_call(L , 0 , 0);
      }
    }

    void LuaObject::RenderUI() {
      if (L == nullptr) {
        return;
      }

      auto render_ui = lua_getglobal(L , "Render_ui");
      if (render_ui == LUA_TFUNCTION) {
        lua_call(L , 0 , 0);
      }
    }

    void LuaObject::Shutdown() {
      if (L == nullptr) {
        return;
      }

      auto shutdown = lua_getglobal(L , "Shutdown");
      if (shutdown == LUA_TFUNCTION) {
        lua_call(L , 0 , 0);
      }

      lua_close(L);
      L = nullptr;
    }

} // namespace other
