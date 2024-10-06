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
    
      LuaObject(ScriptModule* module , UUID handle , const std::string& name , sol::state& script_state , sol::table&& object) 
            : ScriptObjectHandle(LanguageModuleType::LUA_MODULE , module , handle , name) , 
              state(script_state) , object(std::move(object)) {}
      virtual ~LuaObject() override {}

      template <typename R>
      R DefaultReturn() {
        if constexpr (std::same_as<R , void>) {
          return;
        } else {
          return R{};
        }
      }
      
      template <typename R , typename... Args>
      constexpr auto CallMethod(const std::string_view name , Args&&... args) -> R {
        try {
          if (!object.valid()) {
            OE_ERROR("Lua state or object is invalid");
            return DefaultReturn<R>();
          }

          sol::optional<sol::function> function = object[name];
          if (!function.has_value()) {
            OE_ERROR("Could not find function {} in Lua script {}" , name , script_name);
          return DefaultReturn<R>();
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
          return DefaultReturn<R>();
        }
      }
      
      template <typename T>
      void SetField(const std::string& name , T&& arg) {
        try {
          object[name] = arg;
        } catch (const std::exception& e) {
          OE_ERROR("Failed to set field {} in Lua script {}" , name , script_name);
        }
      }

      template <typename R>
      R GetField(const std::string& name) const {
        try {
          return object[name];
        } catch (const std::exception& e) {
          OE_ERROR("Failed to get field {} in Lua script {}" , name , script_name);
          return DefaultReturn<R>();
        }
      }

      template <typename T>
      void SetProperty(const std::string_view name , T&& arg) {
        try {
          object[name] = arg;
        } catch (const std::exception& e) {
          OE_ERROR("Failed to set property {} in Lua script {}" , name , script_name);
        }
      }

      template <typename R>
      R GetProperty(const std::string_view name) const {
        try {
          return object[name];
        } catch (const std::exception& e) {
          OE_ERROR("Failed to get property {} in Lua script {}" , name , script_name);
          return DefaultReturn<R>();
        }
      }

      virtual void InitializeScriptMethods() override;
      virtual void InitializeScriptFields() override;
      virtual void UpdateNativeFields() override {}

    protected:
      sol::state& state;
      sol::table object;
  };


} // namespace other

#endif // !LUA_OBJECT_HPP
