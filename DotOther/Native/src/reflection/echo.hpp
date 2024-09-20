/**
 * \file reflection/echo.hpp
 **/
#ifndef DOTOTHER_ECHO_HPP
#define DOTOTHER_ECHO_HPP

#include "reflection/echo_type.hpp"

namespace dotother {
namespace echo {

  template <typename T , typename IOS>
  void SerializeObject(IOS& stream , T&& object) {}

  template <typename T, typename IOS>
    requires echo::is_serializable_v<T> && echo::has_serializer_v<T>
  void SerializeObject(IOS& stream , T&& object) {
    if (!T::Write(stream, object)) {
      util::print(DO_STR("Failed to serialize object {}"), refl::reflect<T>().name);
    }
  }

  template <typename T, typename IOS>
    requires (echo::is_serializable_v<T> && !echo::has_serializer_v<T>)
  void SerializeObject(IOS& stream , T&& object) {
    refl::util::for_each(refl::reflect<T>(object).members , [&](auto m) {
      if constexpr (is_serializable_v<T>) {
        stream << refl::descriptor::get_display_name(m) << " = ";
        stream << m(object) << "\n";
      }
    }); 
  }

  template <typename T, typename R>
    requires (echo::is_callable_v<T, R> && !echo::is_static_callable_v<T, R>)
  R InvokeCallable(T& object) {
    return object();
  }

  template <typename T, typename R>
    requires echo::is_static_callable_v<T, R>
  R InvokeStaticCallable() {
    return T::Call();
  }

} // namespace echo

  template <typename T, typename IOS>
  void SerializeObject(IOS& stream , T&& object) {
    if constexpr (!echo::is_serializable_v<T>) {
      return;
    }
    echo::SerializeObject<T, IOS>(stream, std::forward<T>(object));
  }

  template <typename T, typename R = void>
    requires (!echo::is_static_callable_v<T, R> && !echo::is_callable_v<T, R>)
  R Invoke() {
    if constexpr (std::same_as<R, void>) {
      return;
    }

    return R{};
  }

  template <typename T, typename R = void>
    requires (!echo::is_static_callable_v<T, R> && !echo::is_callable_v<T, R>)
  R Invoke(T& object) {
    if constexpr (std::same_as<R, void>) {
      return;
    }

    return R{};
  }
  
  template <typename T, typename R = void>
    requires echo::is_static_callable_v<T, R>
  R Invoke() {
    return echo::InvokeStaticCallable<T, R>();
  }

  template <typename T, typename R = void>
    requires (echo::is_callable_v<T, R> && !echo::is_static_callable_v<T, R>)
  R Invoke(T& object) {
    return echo::InvokeCallable<T, R>(object);
  }

} // namespace dotother

#endif // !DOTOTHER_ECHO_HPP
