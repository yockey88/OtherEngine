/**
 * \file object.hpp
 **/
#ifndef DOTOTHER_OBJECT_HPP
#define DOTOTHER_OBJECT_HPP

#include "defines.hpp"
#include <string_view>
#include <utility>
namespace dotother {

  class Object {
    public:
      template <typename Ret , typename... Args>
      Ret Invoke(const std::string_view name, Args&&... params) {
        constexpr size_t param_count = sizeof...(Args);
        Ret res;

        if constexpr (param_count > 0) {
            const void* param_vals[param_count] = {0};
            ManagedType param_types[param_count] = {ManagedType::UNKNOWN};
            util::AddToArrayAt<Args...>(param_vals , param_types , 
                                        std::forward<Args>(params)..., std::make_index_sequence<param_count>{});
            InvokeMethodRetInternal(name, param_vals, param_types, param_count, &res);
        } else {

        }
      }

    private:
      void InvokeMethodRetInternal(const std::string_view name, const void** args, ManagedType* param_types, size_t param_count, void* ret);
  };

} // namespace dotother

#endif // !DOTOTHER_OBJECT_HPP