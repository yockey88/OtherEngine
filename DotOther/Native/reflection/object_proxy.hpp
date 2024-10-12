/**
 * \file reflection/object_proxy.hpp
 **/
#ifndef DOTOTHER_REFLECTION_OBJECT_PROXY_HPP
#define DOTOTHER_REFLECTION_OBJECT_PROXY_HPP

#include <string>
#include <string_view>

#include <refl/refl.hpp>

namespace dotother {
namespace echo {  

  template <typename T>
  struct ObjectProxy : refl::runtime::proxy<ObjectProxy<T>, T> {
    T& target;

    ObjectProxy(T* object_handle) 
    : target(*object_handle) {}

    void InvokeMethod(const std::string_view method_name) {
      std::string mname{ method_name };
      refl::runtime::invoke<void , T>(std::forward<T>(target), mname.c_str());
    }

    template <typename Member , typename Self , typename... Args>
    static constexpr decltype(auto) invoke_impl(Self&& self, Args&&... args) {
      constexpr Member member{};
      if constexpr (refl::descriptor::is_field(member)) {
        static_assert(sizeof...(Args) <= 1 , "Invalid number of arguments provided for property!");

        if constexpr (sizeof...(Args) == 1) {
          static_assert(refl::descriptor::is_writable(member));
          return member(self.target , std::forward<Args>(args)...);
        } else {
          static_assert(refl::descriptor::is_readable(member));
          return refl::util::make_const(member(self.target()));
        }
      } else {
        return member(self.target , std::forward<Args>(args)...);
      }
    }
  }; 

} // namespace echo
} // namespace dotother

#endif // !DOTOTHER_REFLECTION_OBJECT_PROXY_HPP