/**
 * \file reflection/echo.hpp
 **/
#ifndef DOTOTHER_ECHO_HPP
#define DOTOTHER_ECHO_HPP

#include "core/utilities.hpp"
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
  
  template <typename T, typename R, typename... Args>
    requires echo::is_static_callable_v<T, R, Args...>
  static constexpr R InvokeStaticCallable(Args&&... args) {
    if constexpr (std::is_void_v<R> && sizeof...(Args) == 0) {
      T::Call();
    } else if constexpr (std::is_void_v<R>) {
      T::Call(std::forward<Args>(args)...);
    } else if constexpr (sizeof...(Args) == 0) {
      return T::Call();
    } else {
      return T::Call(std::forward<Args>(args)...);
    }
  }

  template <typename T, typename R, typename... Args>
    requires echo::is_callable_v<T, R, Args...>
  static constexpr R InvokeCallable(T& object, Args&&... args) {
    if constexpr (std::is_void_v<R> && sizeof...(Args) == 0) {
      object();
    } else if constexpr (std::is_void_v<R>) {
      object(std::forward<Args>(args)...);
    } else if constexpr (sizeof...(Args) == 0) {
      return object();
    } else {
      return object(std::forward<Args>(args)...);
    }
  }

} // namespace echo

  template <typename T, typename IOS>
  void SerializeObject(IOS& stream , T&& object) {
    if constexpr (!echo::is_serializable_v<T>) {
      return;
    }
    echo::SerializeObject<T, IOS>(stream, std::forward<T>(object));
  }
  
  template <typename T, typename R, typename... Args>
    requires echo::is_static_callable_v<T, R, Args...>
  struct StaticInvoker {
    R operator()(Args&&... args) {
      return echo::InvokeStaticCallable<T, R, Args...>(std::forward<Args>(args)...);
    }
  };

  template <typename T, typename R, typename... Args>
    requires echo::is_callable_v<T, R, Args...>
  struct Invoker {
    R operator()(T& object, Args&&... args) {
      return echo::InvokeCallable<T, R, Args...>(object, std::forward<Args>(args)...);
    }
  };


  template <typename T, typename R = void, typename... Args>
    requires echo::is_callable_v<T, R, Args...>
  R Invoke(T& object, Args&&... args) {
    return Invoker<T, R, Args...>{}(object, std::forward<Args>(args)...);
  }

  template <typename T , typename R = void , typename... Args>
    requires echo::is_static_callable_v<T, R, Args...>
  R Invoke(Args&&... args) {
    return StaticInvoker<T, R, Args...>{}(std::forward<Args>(args)...);
  }

} // namespace dotother

#endif // !DOTOTHER_ECHO_HPP
