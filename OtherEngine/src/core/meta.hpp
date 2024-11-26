/**
 * \file core/meta.hpp
 **/
#ifndef OTHER_ENGINE_META_HPP
#define OTHER_ENGINE_META_HPP

#include <utility>
#include <variant>

#include <spdlog/fmt/fmt.h>

namespace other {
  namespace meta {

    template <typename T>
    struct contains_t {};

    template <typename>
    struct tag_t {};

    template <typename... Ts>
    struct unique_type_list_t : contains_t<Ts>... {
      using variant_t = std::variant<Ts...>;
      using tuple_t = std::tuple<Ts...>;

      template <typename NT>
      struct insert {
        static unique_type_list_t<Ts...> type_(contains_t<NT>*);
        static unique_type_list_t<Ts..., NT> type_(...);

        using type = decltype(type_(std::declval<unique_type_list_t<Ts...>*>()));
      };

      template <typename NT>
      typename insert<NT>::type operator+(NT);
    };

  }  // namespace meta

  template <typename T>
  concept is_enum = std::is_enum_v<T>;

  template <typename E>
    requires is_enum<E>
  constexpr std::underlying_type_t<E> ValOf(E e) {
    /// solve really annoying LSP issue
    return fmt::underlying(e);
  }

  template <typename T>
  concept is_container = requires(T a) {
    typename T::value_type;
    typename T::iterator;
    typename T::const_iterator;
    typename T::size_type;

    { a.begin() } -> std::same_as<typename T::iterator>;
    { a.end() } -> std::same_as<typename T::iterator>;
    { a.cbegin() } -> std::same_as<typename T::const_iterator>;
    { a.cend() } -> std::same_as<typename T::const_iterator>;
    { a.size() } -> std::same_as<typename T::size_type>;
    { a.empty() } -> std::convertible_to<bool>;
  };

  template <typename T>
  concept not_container = !is_container<T>;

}  // namespace other

#endif  // !OTHER_ENGINE_META_HPP