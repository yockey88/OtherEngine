/**
 * \file hosting/native_object.hpp
 **/
#ifndef DOTOTHER_NATIVE_OBJECT_HPP
#define DOTOTHER_NATIVE_OBJECT_HPP

#include <cstdint>

#include <refl/refl.hpp>

#include "reflection/echo_defines.hpp"
#include "reflection/reflected_object.hpp"

namespace dotother {
  namespace echo {

    template <typename T>
    struct ObjectProxy;

  }  // namespace echo

  class NObject : echo::reflectable {
    ECHO_REFLECT();

   public:
    NObject(uint64_t handle);
    virtual ~NObject();

    uint64_t object_handle = 0;
    echo::ObjectProxy<NObject>* proxy = nullptr;
  };

  static_assert(sizeof(NObject) == 24, "Invalid size for NObject!");

}  // namespace dotother

ECHO_TYPE(
  type(
    dotother::NObject,
    refl::attr::bases<dotother::echo::reflectable>
  ),
  field(object_handle),
  field(proxy)
);

#endif  // !DOTOTHER_NATIVE_OBJECT_HPP
