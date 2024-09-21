/**
 * \file hosting/native_object.hpp
 **/
#ifndef DOTOTHER_NATIVE_OBJECT_HPP
#define DOTOTHER_NATIVE_OBJECT_HPP

#include <refl/refl.hpp>

#include "core/utilities.hpp"

#include "reflection/echo_defines.hpp"
#include "reflection/serializable.hpp"

namespace dotother {
namespace echo {

  template <typename T>
  struct ObjectProxy : refl::runtime::proxy<ObjectProxy<T>, T> {
    template <typename Member , typename Self , typename... Args>
    static constexpr decltype(auto) invoke_impl(Self&& self, Args&&... args) {
      constexpr Member member{};
      util::print(DO_STR("Calling {}") , refl::descriptor::get_debug_name(member));

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

  class NObject : echo::serializable {
    ECHO_REFLECT();
    public:
      NObject() {}
      ~NObject() {}

      uint64_t handle;

    private:
  };

} // namespace dotother

ECHO_TYPE(
  type(dotother::NObject) ,
  field(handle , dotother::echo::serializable_field())
);

#endif // !DOTOTHER_NATIVE_OBJECT_HPP
