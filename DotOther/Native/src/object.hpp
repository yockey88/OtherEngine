/**
 * \file object.hpp
 **/
#ifndef DOTOTHER_OBJECT_HPP
#define DOTOTHER_OBJECT_HPP

#include <string_view>

#include "defines.hpp"
#include "utilities.hpp"

namespace dotother {

  class Assembly;
  class Type;

  class Object {
    public:
      template <typename... Args>
      void Invoke(const std::string_view name, Args&&... params) {
        constexpr size_t argc = sizeof...(params);

        if constexpr (argc > 0) {
          const void* params[argc] = {0};
          ManagedType param_types[argc] = {0};

          util::AddToArray<Args...>(params, param_types, std::forward<Args>(params)..., std::make_index_sequence<argc>{});
          InvokeMethod(name, params, param_types, argc, nullptr);
        } else {
          InvokeMethod(name, nullptr, nullptr, 0);
        }
      }

      template <typename Ret , typename... Args>
      Ret Invoke(const std::string_view name, Args&&... params) {
        constexpr size_t argc = sizeof...(params);

        Ret res;
        if constexpr (argc > 0) {
          const void* params[argc] = {0};
          ManagedType param_types[argc] = {0};

          util::AddToArray<Args...>(params, param_types, std::forward<Args>(params)..., std::make_index_sequence<argc>{});
          InvokeReturningMethod(name, params, param_types, argc, &res);
        } else {
          InvokeReturningMethod(name, nullptr, nullptr, 0, &res);
        }
      }


    private:
      /// the .NET object existing in the managed runtime
      void* managed_handle = nullptr;
      Type* type = nullptr;

      void InvokeMethod(std::string_view method_name, const void** params, const ManagedType* types, size_t argc);
      void InvokeReturningMethod(std::string_view method_name, const void** params, const ManagedType* types, 
                                 size_t argc, void* ret);


      friend class Host;
      friend class ManagedAssembly;
      friend class Type;
  };

} // namespace dotother

#endif // !DOTOTHER_OBJECT_HPP
