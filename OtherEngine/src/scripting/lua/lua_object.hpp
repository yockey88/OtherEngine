/**
 * \file scripting/lua/lua_object.hpp
 **/
#ifndef LUA_OBJECT_HPP
#define LUA_OBJECT_HPP

#include <sol/sol.hpp>
#include <string>

#include "scripting/script_object.hpp"

namespace other {

  class LuaObject : public ScriptObjectHandle<LuaObject> {
    public:
    
      LuaObject(ScriptModule* module , UUID handle , const std::string& name , sol::state* script_state , sol::table& object) 
            : ScriptObjectHandle(LanguageModuleType::LUA_MODULE , module , handle , name) , 
              state(script_state) , object(std::move(object)) {}
      virtual ~LuaObject() override {}
      
      template <typename R , typename... Args>
      R CallMethod(const std::string_view name , Args&&... args) {
        try {
          sol::optional<sol::function> function = (*state)[name];
          if (!function.has_value()) {
            function = object[name];

            if (!function.has_value()) {
              OE_ERROR("Could not find function {} in Lua script {}" , name , script_name);
              
              if constexpr (std::same_as<R , void>) {
                return;
              } else {
                return R{};
              }
            }
          }

            
          sol::function func = *function;
          if constexpr (sizeof...(Args) == 0) {
            if constexpr (std::same_as<R , void>) {
              func();
            } else {
              return func();
            }
          } else {
            if constexpr (std::same_as<R , void>) {
              func(std::forward<Args>(args)...);
            } else {
              return func(std::forward<Args>(args)...);
            }
          }
        } catch (const std::exception& e) {
          OE_ERROR("Lua error caught calling function {} in script {}" , name , script_name);
          return R{};
        }
      }

      virtual void InitializeScriptMethods() override;
      virtual void InitializeScriptFields() override;
      virtual void UpdateNativeFields() override {}
      
      virtual Opt<Value> GetField(const std::string& name) override;
      virtual void SetField(const std::string& name , const Value& value) override;

    protected:
      sol::state* state = nullptr;
      sol::table object;
  
      void OnSetEntityId() override;
  };


} // namespace other

#endif // !LUA_OBJECT_HPP
