/**
 * \file memory/buffer_allocator.hpp
 **/
#ifndef OTHERE_ENGINE_BUFFER_ALLOCATOR_HPP
#define OTHERE_ENGINE_BUFFER_ALLOCATOR_HPP

#include <cstdint>
#include <cstring>
#include <new>
#include <utility>

#include "profiling/profiling.hpp"

#include "memory/allocator.hpp"
#include "memory/arena.hpp"

namespace other {

  /**
   * @note it is important that BufferALlocator (and most of the other allocators) do not maintain any state,
   *          these need to be completely stateless so that anyone with memory allocated from the arena can free it without
   *          needing the original allocator that allocated the memory
   **/

  class BufferAllocator : public Allocator {
   public:
    BufferAllocator() = default;
    virtual ~BufferAllocator() override = default;

    constexpr BufferAllocator(const BufferAllocator&) noexcept {}

    uint8_t* Allocate(size_t length) {
      PROFILE_SECTION("BufferAllocator--Allocate");
      /// TODO: custom alignment
      void* memory = Arena::Allocate(length);
      if (memory == nullptr) {
        throw std::bad_alloc();
      }
      PROFILE_ALLOCATION(memory, length);
      new (memory) uint8_t[length];
      std::memset(memory, 0, length);
      return std::launder(reinterpret_cast<uint8_t*>(memory));
    }

    void* AllocateBlock(size_t size) {
      return Arena::Allocate(size);
    }

    void Free(uint8_t* ptr, size_t length) {
      PROFILE_SECTION("BufferAllocator--Free");

      if (ptr != nullptr) {
        PROFILE_DEALLOCATION(ptr);
        std::memset(ptr, 0, length);
      }
      Arena::Free(ptr, length);
    }

    void Free(void* ptr, size_t length) {
      if (ptr != nullptr) {
        PROFILE_DEALLOCATION(ptr);
        uint8_t* t_ptr = static_cast<uint8_t*>(ptr);
        std::memset(t_ptr, 0, length);
      }
      Arena::Free(ptr, length);
    }
  };

  inline bool operator==(const BufferAllocator&, const BufferAllocator&) { return true; }
  inline bool operator!=(const BufferAllocator&, const BufferAllocator&) { return false; }

}  // namespace other

#endif  // !OTHERE_ENGINE_BUFFER_ALLOCATOR_HPP
