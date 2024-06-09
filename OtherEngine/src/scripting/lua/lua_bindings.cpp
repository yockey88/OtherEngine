/**
 * \file scripting/ecs/lua_bindings.cpp
 **/
#include "scripting/lua/lua_bindings.hpp"

#include "core/defines.hpp"

namespace other {

  void LuaScriptBindings::InitializeBindings(sol::state &lua_state) {
    lua_state["Logger"]["WriteTrace"] = [&](const std::string& str) {
      println("[ Lua Trace ]> {}" , str);
    };
    lua_state["Logger"]["WriteDebug"] = [&](const std::string& str) {
      println("[ Lua Debug ]> {}" , str);
    };
    lua_state["Logger"]["WriteInfo"] = [&](const std::string& str) {
      println("[ Lua Info ]> {}" , str);
    };
    lua_state["Logger"]["WriteWarn"] = [&](const std::string& str) {
      println("[ Lua Warning ]> {}" , str);
    };
    lua_state["Logger"]["WriteError"] = [&](const std::string& str) {
      println("[ Lua Error ]> {}" , str);
    };
    lua_state["Logger"]["WriteCritical"] = [&](const std::string& str) {
      println("[ Lua Critical ]> {}" , str);
    };
  }

} // namespace other
