/**
 * \file scripting/lua/lua_script.hpp
 **/
#ifndef LUA_SCRIPT_HPP
#define LUA_SCRIPT_HPP

#include <string>

#include "scripting/script_module.hpp"

#include <sol/sol.hpp>

#include "lua_object.hpp"

namespace other {

  class LuaScript : public ScriptModule {
    public:
      LuaScript(const std::vector<std::string>& paths);
      virtual ~LuaScript() override;

      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") override;

    private:
      sol::state lua_state;

      std::map<UUID , LuaObject> loaded_objects;

      std::vector<std::string> paths;
  };  

} // namespace other

#endif // !LUA_SCRIPT_HPP
