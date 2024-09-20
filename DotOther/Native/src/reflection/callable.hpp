/**
 * \file reflection/callable.hpp
 **/
#ifndef DOTOTHER_CALLABLE_HPP
#define DOTOTHER_CALLABLE_HPP

#include <type_traits>

#include <refl/refl.hpp>

#include "reflection/reflected_object.hpp"

namespace dotother {
namespace echo {

  struct callable : reflectable, echo_attr, 
                    refl::attr::usage::function {};

  struct static_callable : callable {};

  template <typename T>
  concept function_t = 
    std::conjunction_v<refl::trait::is_function<T>, refl::trait::detail::is_function_2<T>>;
  
  template <typename T, typename R>
  concept callable_t = (std::derived_from<T , callable> || function_t<T>) && 
    requires (T t) {
      t();
      { t() } -> std::convertible_to<R>;
    };

  template <typename T, typename R>
  concept static_callable_t = std::derived_from<T , static_callable> &&
    requires (T t) {
      T::Call();
      { T::Call() } -> std::convertible_to<R>;
    };

} // namespace echo
} // namespace dotother

#endif // !DOTOTHER_CALLABLE_HPP
