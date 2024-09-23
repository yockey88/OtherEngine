/**
 * \file reflection/echo_type.hpp
 **/
#include "reflection/reflected_object.hpp"

#include <refl/refl.hpp>

#include "reflection/serializable.hpp"
#include "reflection/callable.hpp"

namespace echo = dotother::echo;

namespace dotother {
namespace echo {

  template <typename T>
  concept reflected_t = std::derived_from<T, reflectable>;
  
  template <typename T>
  concept echo_attr_t = std::derived_from<T, echo_attr>;
  
  template <typename T, typename R = void, typename... Args>
  struct echo_traits {
    constexpr static bool is_echo_type = reflected_t<T>;
    constexpr static bool is_echo_attr = echo_attr_t<T>;

    constexpr static bool is_field = field_t<T>;
    constexpr static bool is_serializable = serializable_t<T>;
    constexpr static bool is_serializable_field = serializable_field_t<T>;
    constexpr static bool has_serializer = has_serializer_t<T>;

    constexpr static bool is_callable_void = callable_t<T, R>;
    constexpr static bool is_callable_args = callable_t<T, R, Args...>;
    constexpr static bool is_static_callable_void = static_callable_t<T, R>;
    constexpr static bool is_static_callable_args = static_callable_t<T, R, Args...>;
  };

  template <typename T>
  constexpr static bool is_echo_type_v = echo_traits<T>::is_echo_type;
  template <typename T>
  constexpr static bool is_echo_attr_v = echo_traits<T>::is_echo_attr;
  
  template <typename T>
  constexpr static bool is_field_v = echo_traits<T>::is_field;
  template <typename T>
  constexpr static bool is_serializable_v = echo_traits<T>::is_serializable;
  template <typename T>
  constexpr static bool is_serializable_field_v = echo_traits<T>::is_serializable_field;
  template <typename T>
  constexpr static bool has_serializer_v = echo_traits<T>::has_serializer;

  template <typename T, typename R>
  constexpr static bool is_callable_void_v = echo_traits<T, R>::is_callable_void;
  template <typename T, typename R, typename... Args>
  constexpr static bool is_callable_args_v = echo_traits<T, R, Args...>::is_callable_args;

  template <typename T, typename R>
  constexpr static bool is_static_callable_void_v = echo_traits<T, R>::is_static_callable_void;
  template <typename T, typename R, typename... Args>
  constexpr static bool is_static_callable_args_v = echo_traits<T, R, Args...>::is_static_callable_args;
  
  template <typename T, typename R, typename... Args>
  constexpr static bool is_callable_v = is_callable_void_v<T, R> || is_callable_args_v<T , R, Args...>;

  template <typename T, typename R, typename... Args>
  constexpr static bool is_static_callable_v = is_static_callable_void_v<T, R> || is_static_callable_args_v<T, R, Args...>;

} // namespace echo
} // namespace dotother
