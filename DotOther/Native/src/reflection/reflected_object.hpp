/**
 * \file reflection/reflected_object.hpp
 **/
#ifndef DOTOTHER_REFLECTED_OBJECT_HPP
#define DOTOTHER_REFLECTED_OBJECT_HPP

#include "reflection/type_database.hpp"
#include "reflection/echo_defines.hpp"

#define ECHO_REFLECT() \
  public: \
    virtual const dotother::echo::TypeMetadata& ReadMetadata() const override { \
      return dotother::echo::TypeDatabase::Instance().Get< \
        refl::trait::remove_qualifiers_t<decltype(*this)> \
      >(); \
    } \

namespace dotother {
namespace echo {

  struct reflectable {
    virtual const TypeMetadata& ReadMetadata() const = 0;
  };

  struct echo_attr {};
 
} // namespace echo
} // namespace dotother

ECHO_TYPE(
  type(dotother::echo::reflectable , refl::attr::bases<>)
);

#endif // DOTOTHER_REFLECTED_OBJECT_HPP
