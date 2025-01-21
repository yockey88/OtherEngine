/**
 * \file reflection/object_proxy.hpp
 **/
#ifndef DOTOTHER_REFLECTION_OBJECT_PROXY_HPP
#define DOTOTHER_REFLECTION_OBJECT_PROXY_HPP

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <string>
#include <string_view>

#include <refl/refl.hpp>

#include "core/dotother_defines.hpp"
#include "core/utilities.hpp"

namespace dotother {
  namespace echo {

    template <typename T>
    struct ObjectProxy : refl::runtime::proxy<ObjectProxy<T>, T> {
      T& target;
      refl::descriptor::type_descriptor<T> descriptor{};
      opt<std::string> cached_name = std::nullopt;

      ObjectProxy(T* object_handle)
          : target(*object_handle) {}

      ObjectProxy(ObjectProxy&& other)
          : target(other.target) {}
      ObjectProxy(const ObjectProxy& other)
          : target(other.target) {}
      ObjectProxy& operator=(ObjectProxy&& other) {
        target = other.target;
        return *this;
      }
      ObjectProxy& operator=(const ObjectProxy& other) {
        target = other.target;
        return *this;
      }

      ObjectProxy Clone() const {
        return ObjectProxy(&target);
      }

      std::string GetTypeName() const {
        return (std::string)refl::descriptor::get_name(descriptor);
      }

      template <typename... Args>
      void InvokeMethod(const std::string_view method_name, Args&&... args) {
        try {
          std::string mname{ method_name };

          if (sizeof...(Args) == 0) {
            refl::runtime::invoke<void, T>(std::forward<T>(target), mname.c_str());
          } else {
            refl::runtime::invoke<void, T>(std::forward<T>(target), mname.c_str(), std::forward<Args>(args)...);
          }
        } catch (const std::exception& e) {
          DOTOTHER_LOG(DO_STR("Exception caught in InvokeMethod: "), MessageLevel::ERR, e.what());
        }
      }

      template <typename R, typename... Args>
      R InvokeMethod(const std::string_view method_name, Args&&... args) {
        try {
          std::string mname{ method_name };

          if (sizeof...(Args) == 0) {
            return refl::runtime::invoke<R, T>(std::forward<T>(target), mname.c_str());
          } else {
            return refl::runtime::invoke<R, T>(std::forward<T>(target), mname.c_str(), std::forward<Args>(args)...);
          }
        } catch (const std::exception& e) {
          DOTOTHER_LOG(DO_STR("Exception caught in InvokeMethod: "), MessageLevel::ERR, e.what());
          return DefaultReturn<R>();
        }
      }

      template <typename Member, typename Self, typename... Args>
      static constexpr decltype(auto) invoke_impl(Self&& self, Args&&... args) {
        constexpr Member member{};
        if constexpr (refl::descriptor::is_field(member)) {
          static_assert(sizeof...(Args) <= 1, "Invalid number of arguments provided for property!");

          if constexpr (sizeof...(Args) == 1) {
            static_assert(refl::descriptor::is_writable(member));
            return member(self.target, std::forward<Args>(args)...);
          } else {
            static_assert(refl::descriptor::is_readable(member));
            return refl::util::make_const(member(self.target()));
          }
        } else {
          return member(self.target, std::forward<Args>(args)...);
        }
      }

     private:
      template <typename R>
      R DefaultReturn() {
        if constexpr (std::is_void_v<R>) {
          return;
        } else {
          return R{};
        }
      }
    };

  }  // namespace echo
}  // namespace dotother

#endif  // !DOTOTHER_REFLECTION_OBJECT_PROXY_HPP