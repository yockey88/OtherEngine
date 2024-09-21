/**
 * \file hosting/native_object.hpp
 **/
#ifndef DOTOTHER_NATIVE_OBJECT_HPP
#define DOTOTHER_NATIVE_OBJECT_HPP

#include <refl/refl.hpp>

namespace dotother {
namespace echo {

  template <typename T>
  struct ObjectProxy : refl::runtime::proxy<ObjectProxy<T>, T> {
    T target;

    constexpr ObjectProxy(const T& target) 
      : target(target) {}

    constexpr ObjectProxy(T&& target)
      : target(std::move(target)) {}

    template <typename... Args>
    constexpr ObjectProxy(Args&&... args)
      : target(std::forward<Args>(args)...) {}

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
// namespace detail {

  class NObject {
    // ECHO_REFLECT();
    public:
      NObject() {}
      ~NObject() {}

      uint64_t handle;

    private:
  };

// } // namespace detail
} // namespace dotother


// ECHO_TYPE(
//   type(dotother::detail::NObject) ,
//   field(handle)
// );

namespace dotother {

  // using NObject = echo::ObjectProxy<detail::NObject>;

} // namespace dotother

#endif // !DOTOTHER_NATIVE_OBJECT_HPP
