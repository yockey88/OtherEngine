/**
 * \file hosting/field.hpp
 **/
#ifndef DOTOTHER_FIELD_HPP
#define DOTOTHER_FIELD_HPP

#include <string_view>
#include <vector>

#include "core/dotother_defines.hpp"
#include "core/utilities.hpp"

#include "hosting/native_string.hpp"

namespace dotother {

  class Type;
  class Attribute;

  class Field {
   public:
    Field(uint32_t handle);
    ~Field() = default;

    NString GetName() const;
    Type& GetType();

    TypeAccessibility Accessibility() const;

    std::vector<Attribute> Attributes() const;

   private:
    int32_t handle = -1;
    Type* type = nullptr;

    friend class Type;
  };

}  // namespace dotother

#endif  // !DOTOTHER_FIELD_HPP
