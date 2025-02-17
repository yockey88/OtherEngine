/**
 * \file hosting/attribute.hpp
 **/
#ifndef DOTOTHER_ATTRIBUTE_HPP
#define DOTOTHER_ATTRIBUTE_HPP

#include <cstdint>

namespace dotother {

  class Type;

  class Attribute {
   public:
    Attribute(uint32_t handle);

    Type& GetType();

   private:
    int32_t handle;
  };

}  // namespace dotother

#endif  // !DOTOTHER_ATTRIBUTE_HPP
