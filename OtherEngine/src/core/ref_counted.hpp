/**
 * \file core/ref_counted.hpp
 */
#ifndef OTHER_ENGINE_REF_COUNTED_HPP
#define OTHER_ENGINE_REF_COUNTED_HPP

#include <atomic>

namespace other {

  class RefCounted {
    public:
      RefCounted() 
        : count(0) {}
      virtual ~RefCounted() = default;

      void Increment();
      void Decrement();

      uint64_t Count() const { return count; }
    
    private:
      mutable std::atomic<uint64_t> count;
  };

} // namespace other

#endif // !OTHER_ENGINE_REF_COUNTED_HPP
