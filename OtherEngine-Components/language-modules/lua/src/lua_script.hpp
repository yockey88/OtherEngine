/**
 * \file lua_script.hpp
 **/
#ifndef LUA_SCRIPT_HPP
#define LUA_SCRIPT_HPP

#include <string>

#include "scripting/script_module.hpp"

extern "C" {
  #include <lua/lua.h>
  #include <lua/lualib.h>
  #include <lua/lauxlib.h>
}

#include "lua_object.hpp"

namespace other {

  class LuaScript : public ScriptModule {
    public:
      LuaScript(Engine* engine , const std::string& path);
      virtual ~LuaScript() override;

      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") override;

    private:
      std::map<UUID , LuaObject> loaded_objects;

      std::string path;
      std::vector<std::string> script_paths;
  };  

} // namespace other

#endif // !LUA_SCRIPT_HPP
