/**
 * \file core/meta.hpp
 **/
#ifndef OTHER_ENGINE_META_HPP
#define OTHER_ENGINE_META_HPP

#include <cmath>
#include <utility>
#include <variant>

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
}  // namespace other

#endif  // !OTHER_ENGINE_META_HPP