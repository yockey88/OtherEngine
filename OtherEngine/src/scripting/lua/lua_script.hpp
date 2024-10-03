/**
 * \file scripting/lua/lua_script.hpp
 **/
#ifndef LUA_SCRIPT_HPP
#define LUA_SCRIPT_HPP

#include <string>

#include <sol/sol.hpp>

#include "core/ref.hpp"

#include "scripting/script_module.hpp"
#include "scripting/lua/lua_object.hpp"

namespace other {

  class LuaScript : public ScriptModule {
    public:
      LuaScript(const std::string& path , const std::string& module_name) 
        : ScriptModule(LanguageModuleType::LUA_MODULE , module_name) , path(path) {}
      virtual ~LuaScript() override {}
      
      template <typename R , typename... Args>
      R CallMethod(const std::string_view name , Args&&... args) {
        // try {
        //   sol::optional<sol::function> function = (*state)[name];
        //   if (!function.has_value()) {
        //     function = object[name];

        //     if (!function.has_value()) {
        //       OE_ERROR("Could not find function {} in Lua script {}" , name , script_name);
              
        //       if constexpr (std::same_as<R , void>) {
        //         return;
        //       } else {
        //         return R{};
        //       }
        //     }
        //   }

            
        //   sol::function func = *function;
        //   if constexpr (sizeof...(Args) == 0) {
        //     if constexpr (std::same_as<R , void>) {
        //       func();
        //     } else {
        //       return func();
        //     }
        //   } else {
        //     if constexpr (std::same_as<R , void>) {
        //       func(std::forward<Args>(args)...);
        //     } else {
        //       return func(std::forward<Args>(args)...);
        //     }
        //   }
        // } catch (const std::exception& e) {
          // OE_ERROR("Lua error caught calling function {} in script {} : [{}]" , name , script_name , e.what());
          if constexpr (std::same_as<R , void>) {
            return;
          } else {
            return R{};
          }
        // }
      }

      virtual void Initialize() override;
      virtual void Shutdown() override;
      virtual void Reload() override;
      virtual bool HasScript(UUID id) const override;
      virtual bool HasScript(const std::string_view name , const std::string_view nspace = "") const override;
      virtual Ref<ScriptObject> GetScriptObject(const std::string& name , const std::string& nspace) override;
      
      virtual std::vector<ScriptObjectTag> GetObjectTags() override;

    private:
      sol::state lua_state;

      std::vector<std::string> loaded_tables;
      std::map<UUID , Ref<ScriptObjectHandle<LuaObject>>> loaded_objects;

      std::string path;
  };  

} // namespace other

#endif // !LUA_SCRIPT_HPP
