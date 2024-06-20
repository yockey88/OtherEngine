/**
 * \file scripting/lua/lua_module.hpp
 **/
#ifndef LUA_MODULE_HPP
#define LUA_MODULE_HPP

#include <optional>
#include <sol/sol.hpp>

#include "scripting/language_module.hpp"
#include "scripting/script_defines.hpp"

namespace other {
  
  class LuaModule : public LanguageModule {
    public:
      LuaModule() 
          : LanguageModule(LanguageModuleType::LUA_MODULE) {}
      virtual ~LuaModule() override {}

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

      virtual bool Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual ScriptModule* GetScriptModule(const std::string& name) override;
      virtual ScriptModule* GetScriptModule(const UUID& id) override;
      virtual ScriptModule* LoadScriptModule(const ScriptModuleInfo& module_info) override;

      virtual void UnloadScriptModule(const std::string& name) override;

      virtual std::string_view GetModuleName() const override;
      virtual std::string_view GetModuleVersion() const override;

    private:
      constexpr static std::string_view kLuaCorePath = "OtherEngine-ScriptCore/lua";

      sol::state lua_state;

      std::vector<std::string> core_files;
  };

} // namespace other

#endif // !LUA_MODULE_HPP
