/**
 * \file serializable.hpp
 **/
#ifndef DOTOTHER_SERIALIZABLE_HPP
#define DOTOTHER_SERIALIZABLE_HPP

#include <concepts>
#include <type_traits>

#include <refl/refl.hpp>

#include "reflection/reflected_object.hpp"

namespace dotother {
namespace echo {

  struct serializable : reflectable, refl::attr::usage::any {};

  struct serializable_field : echo_attr, refl::attr::usage::field {};

  template <typename T>
  concept field_t = 
    std::conjunction_v<refl::trait::is_member<T>, refl::trait::detail::is_field_2<T>>;

  template <typename T>
  concept serializable_field_t = std::derived_from<T, serializable_field> && field_t<T>;

  template <typename T>
  concept serializable_t = std::derived_from<T, serializable> || serializable_field_t<T>;

  template <typename T>
  concept has_serializer_t = requires {
    { T::Write(std::declval<std::ostream&>(), std::declval<T>()) } -> std::same_as<bool>;
  };

} // namespace echo
} // namespace dotother

#endif // !DOTOTHER_SERIALIZABLE_HPP
