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
      LuaScript(const std::string& path) 
        : ScriptModule(LanguageModuleType::LUA_MODULE) , path(path) {}
      virtual ~LuaScript() override {}
      
      template <typename Ret , typename... Args>
      Opt<Ret> CallLuaMethod(const std::string_view name , Args&&... args) {
        sol::optional<sol::function> func = lua_state[name];
        if (!func.has_value()) {
          return std::nullopt;
        }

        if (!(*func).valid()) {
          return std::nullopt;
        }

        return (*func)(std::forward<Args>(args)...);
      }

      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual bool HasScript(UUID id) override;
      virtual bool HasScript(const std::string_view name , const std::string_view nspace = "") override;
      virtual ScriptObject* GetScript(const std::string& name , const std::string& nspace = "") override;
      
      virtual std::vector<ScriptObjectTag> GetObjectTags() override;

    private:
      sol::state lua_state;

      std::vector<std::string> loaded_tables;
      std::map<UUID , LuaObject> loaded_objects;

      std::string path;
  };  

} // namespace other

#endif // !LUA_SCRIPT_HPP
