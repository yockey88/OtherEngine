/**
 * \file scripting/lua/lua_component_bindings.cpp
 **/
#include "scripting/lua/lua_component_bindings.hpp"

#include <entt/entt.hpp>

namespace other {
namespace lua_script_bindings {

  void BindEcsTypes(sol::state& lua_state) {
    using namespace entt::literals;

    lua_state.new_usertype<entt::registry>(
      "registry" , 
      sol::meta_function::construct , sol::factories([]() -> entt::registry { return entt::registry{}; }) ,
      "size" , [](const entt::registry& self) -> size_t { return self.storage<entt::entity>()->size(); } ,
      "alive" , [](const entt::registry& self) -> bool { return self.storage<entt::entity>()->free_list(); } ,
      "valid" , &entt::registry::valid , 
      "current" , &entt::registry::current , 
      "create" , [](entt::registry& self) -> entt::entity { return self.create(); } ,
      "destroy" , [](entt::registry& self , entt::entity handle) { return self.destroy(handle); } // ,
      /// TODO: implement entt::meta reflection bindings to complete binding ecs to lua 
      // "emplace" , [](entt::registry& self , entt::entity entity , const sol::table& comp , sol::this_state s) -> sol::object {
      //   if (!comp.valid()) {
      //     return sol::lua_nil_t{};
      //   }

      //   const auto maybe_any = InvokeMetaFunc(GetTypeId(comp) , "emplace"_hs , &self , entity , comp , s);
      //   return maybe_any ? 
      //     maybe_any.cast<sol::reference>() : 
      //     sol::lua_nil_t{};
      // }
    );
  }

} // namespace lua_script_bindings  
} // namespace other
