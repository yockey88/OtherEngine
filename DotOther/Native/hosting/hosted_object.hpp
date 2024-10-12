/**
 * \file hosting/hosted_object.hpp
 **/
#ifndef DOTOTHER_HOSTED_OBJECT_HPP
#define DOTOTHER_HOSTED_OBJECT_HPP

#include <string_view>

#include "core/dotother_defines.hpp"
#include "core/utilities.hpp"

namespace dotother {

  class Assembly;
  class Type;

  template <typename T>
  concept PtrType = std::is_pointer_v<T>;

  template <typename T>
  concept NotPtrType = !PtrType<T>;

  class HostedObject {
    public:
      template <typename Ret , typename... Args>
      Ret Invoke(const std::string_view name, Args&&... params) {
        constexpr size_t argc = sizeof...(params);

        if constexpr (std::same_as<Ret , void>) {
          if constexpr (argc > 0) {
            const void* parameters[argc] = {0};
            ManagedType param_types[argc] = {};

            util::AddToArray<Args...>(parameters, param_types, std::forward<Args>(params)..., std::make_index_sequence<argc>{});
            InvokeMethod(name, parameters, param_types, argc);
          } else {
            InvokeMethod(name, nullptr, nullptr, 0);
          }
        } else {
          Ret res;
          if constexpr (argc > 0) {
            const void* parameters[argc] = {0};
            ManagedType param_types[argc] = {};

            util::AddToArray<Args...>(parameters, param_types, std::forward<Args>(params)..., std::make_index_sequence<argc>{});
            InvokeReturningMethod(name, parameters, param_types, argc, &res);
          } else {
            InvokeReturningMethod(name, nullptr, nullptr, 0, &res);
          }
        }
      }

      void SetField(const std::string_view name, PtrType auto value) {
        WriteToField(name, value);
      }

      void SetField(const std::string_view name, NotPtrType auto value) {
        WriteToField(name, &value);
      }

      template <typename T>
      T GetField(const std::string_view name) {
        T res;
        ReadFromField(name, &res);
        return res;
      }

      void SetProperty(const std::string_view name, PtrType auto value) {
        WriteToProperty(name, value);
      }

      void SetProperty(const std::string_view name, NotPtrType auto value) {
        WriteToProperty(name, &value);
      }

      template <typename T>
      T GetProperty(const std::string_view name) {
        T res;
        ReadFromProperty(name, &res);
        return res;
      }

    private:
      /// the .NET object existing in the managed runtime
      void* managed_handle = nullptr;
      Type* type = nullptr;

      void InvokeMethod(std::string_view method_name, const void** params, const ManagedType* types, size_t argc);
      void InvokeReturningMethod(std::string_view method_name, const void** params, const ManagedType* types, 
                                 size_t argc, void* ret);

      void WriteToField(const std::string_view name, void* value);
      void ReadFromField(const std::string_view name, void* value);

      void WriteToProperty(const std::string_view name, void* value);
      void ReadFromProperty(const std::string_view name, void* value);

      friend class Host;
      friend class ManagedAssembly;

      friend class Type;
  };

} // namespace dotother

#endif // !DOTOTHER_HOSTED_OBJECT_HPP
