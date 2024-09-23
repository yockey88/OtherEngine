/**
 * \file hosting/hosted_object.hpp
 **/
#ifndef DOTOTHER_HOSTED_OBJECT_HPP
#define DOTOTHER_HOSTED_OBJECT_HPP

#include <string_view>

#include "core/defines.hpp"
#include "core/utilities.hpp"

namespace dotother {

  class Assembly;
  class Type;

  class HostedObject {
    public:
      template <typename... Args>
      void Invoke(const std::string_view name, Args&&... params) {
        constexpr size_t argc = sizeof...(params);

        if constexpr (argc > 0) {
          const void* parameters[argc] = {0};
          ManagedType param_types[argc] = {};

          util::AddToArray<Args...>(parameters, param_types, std::forward<Args>(params)..., std::make_index_sequence<argc>{});
          InvokeMethod(name, parameters, param_types, argc);
        } else {
          InvokeMethod(name, nullptr, nullptr, 0);
        }
      }

      template <typename Ret , typename... Args>
      Ret Invoke(const std::string_view name, Args&&... params) {
        constexpr size_t argc = sizeof...(params);

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

      template <typename T>
      void SetField(const std::string_view name, T value) {
        WriteToField(name, &value);
      }

      template <typename T>
      T GetField(const std::string_view name) {
        T res;
        ReadFromField(name, &res);
        return res;
      }

      template <typename T>
      void SetProperty(const std::string_view name, T value) {
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
