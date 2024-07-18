/**
 * \file scripting/lua/lua_module.hpp
 **/
#ifndef LUA_MODULE_HPP
#define LUA_MODULE_HPP

#include <sol/sol.hpp>

#include "scripting/language_module.hpp"
#include "scripting/script_defines.hpp"
#include "scripting/lua/lua_script.hpp"

namespace other {
  
  class LuaModule : public LanguageModule {
    public:
      LuaModule() 
          : LanguageModule(LanguageModuleType::LUA_MODULE) {}
      virtual ~LuaModule() override {}

      LuaScript* GetRawScriptHandle(const std::string_view name);

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

      std::vector<std::string> core_files;
  };

} // namespace other

#endif // !LUA_MODULE_HPP
