/**
 * \file memory/allocator.hpp
 **/
#ifndef OTHER_ENGINE_ALLOCATOR_HPP
#define OTHER_ENGINE_ALLOCATOR_HPP

namespace other {

  class Allocator {
   public:
    virtual ~Allocator() = default;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ALLOCATOR_HPP