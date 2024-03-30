/**
 * \file lua_module.hpp
 **/
#ifndef LUA_MODULE_HPP
#define LUA_MODULE_HPP

#include "core/platform.hpp"
#include "scripting/language_module.hpp"

extern "C" {
  #include <lua/lua.h>
  #include <lua/lualib.h>
  #include <lua/lauxlib.h>
}

namespace other {
  
  class LuaModule : public LanguageModule {
    public:
      LuaModule(Engine* engine);
      virtual ~LuaModule() override;

      virtual bool Initialize() override;
      virtual bool Reinitialize() override;
      virtual void Shutdown() override;

      virtual ScriptModule* GetScriptModule(const std::string& name) override;
      virtual ScriptModule* GetScriptModule(const UUID& id) override;
      virtual ScriptModule* LoadScriptModule(const ScriptModuleInfo& module_info) override;

      virtual void UnloadScriptModule(const std::string& name) override;

      virtual std::string_view GetModuleName() const override;
      virtual std::string_view GetModuleVersion() const override;

    private:
      lua_State* L;
      constexpr static luaL_Reg lualibs[] = {
        { "base" , luaopen_base },
        { nullptr , nullptr }
      };

      constexpr static std::string_view kLuaCore = "OtherEngine-Components/language-modules/lua";

      std::filesystem::path lua_core_path;
  };

} // namespace other

OE_API other::Plugin* CreatePlugin(other::Engine* engine);
OE_API void DestroyPlugin(other::Plugin* plugin);

#endif // !LUA_MODULE_HPP
