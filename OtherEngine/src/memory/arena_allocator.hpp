/**
 * \file memory/arena_allocator.hpp
 **/
#ifndef OTHERE_ENGINE_ARENA_ALLOCATOR_HPP
#define OTHERE_ENGINE_ARENA_ALLOCATOR_HPP

#include <cstring>
#include <new>
#include <utility>

#include "profiling/profiling.hpp"

#include "memory/allocator.hpp"
#include "memory/arena.hpp"

namespace other {

  /**
   * @note it is important that ArenaAllocator (and most of the other allocators) do not maintain any state,
   *          these need to be completely stateless so that anyone with memory allocated from the arena can free it without
   *          needing the original allocator that allocated the memory
   **/

  template <typename T>
  class ArenaAllocator : public Allocator {
   public:
    using value_type = T;

    ArenaAllocator() = default;
    virtual ~ArenaAllocator() override = default;

    template <typename U>
    constexpr ArenaAllocator(const ArenaAllocator<U>&) noexcept {}

    template <typename... Args>
    T* Allocate(Args&&... args) {
      PROFILE_SECTION("ArenaAllocator--Allocate");
      /// TODO: custom alignment
      void* memory = Arena::Allocate(type_size);
      if (memory == nullptr) {
        throw std::bad_alloc();
      }
      new (memory) T(std::forward<Args>(args)...);
      return std::launder(static_cast<T*>(memory));
    }

    void* AllocateBlock(size_t size) {
      return Arena::Allocate(size);
    }

    void Free(T* ptr) {
      PROFILE_SECTION("ArenaAllocator--Free");

      if (ptr != nullptr) {
        ptr->~T();
      }
      Arena::Free(ptr, type_size);
    }

    void Free(void* ptr) {
      if (ptr != nullptr) {
        T* t_ptr = std::launder(static_cast<T*>(ptr));
        t_ptr->~T();
      }
      Arena::Free(ptr, type_size);
    }

    /// cpp standard allocator interface
    [[nodiscard]] T* allocate(size_t size) {
      return std::launder(static_cast<T*>(AllocateBlock(size)));
    }

    void deallocate(void* ptr, size_t) noexcept {
      Free(std::launder(static_cast<T*>(ptr)));
    }

    static constexpr size_t type_size = sizeof(T);
  };

  template <class T, class U>
  bool operator==(const ArenaAllocator<T>&, const ArenaAllocator<U>&) {
    return true;
  }

  template <class T, class U>
  bool operator!=(const ArenaAllocator<T>&, const ArenaAllocator<U>&) { return false; }

}  // namespace other

#endif  // !OTHERE_ENGINE_ARENA_ALLOCATOR_HPP
