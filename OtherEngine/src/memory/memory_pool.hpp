/**
 * \file memory/memory_pool.hpp
 **/
#ifndef OTHER_ENGINE_MEMORY_POOL_HPP
#define OTHER_ENGINE_MEMORY_POOL_HPP

#include <cstdint>
#include <cstring>
#include <new>
#include <span>

#include "core/logger.hpp"
#include "core/ref_counted.hpp"

#include "profiling/profiling.hpp"

namespace other {

  template <typename T>
    requires std::default_initializable<T>
  class MemoryPool : public RefCounted {
   public:
    MemoryPool(size_t max_objects)
        : max_objects(max_objects) {
      AllocateBlock();
      for (size_t i = 0; i < max_objects; i++) {
        new (memory + (i * sizeof(T))) T();
      }
    }

    template <typename... Args>
      requires std::constructible_from<T, Args...>
    MemoryPool(size_t max_objects, Args&&... init_args)
        : max_objects(max_objects) {
      AllocateBlock();
      for (size_t i = 0; i < max_objects; i++) {
        new (memory + (i * sizeof(T))) T(std::forward<Args>(init_args)...);
      }
    }

    ~MemoryPool() {
      FreeBlock();
    }

    MemoryPool(MemoryPool&&) = delete;
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(MemoryPool&&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    T& operator[](size_t idx) {
      OE_ASSERT(idx < max_objects, "Index out of bounds");
      /// safe to use temp object because span is a view and does not own the memory
      return Objects()[idx];
    }

    const T& operator[](size_t idx) const {
      OE_ASSERT(idx < max_objects, "Index out of bounds");
      /// safe to use temp object because span is a view and does not own the memory
      return Objects()[idx];
    }

    T* At(size_t idx) {
      OE_ASSERT(idx < max_objects, "Index out of bounds");
      return &Objects()[idx];
    }

    const T* At(size_t idx) const {
      OE_ASSERT(idx < max_objects, "Index out of bounds");
      return &Objects()[idx];
    }

    const size_t MaxObjects() const {
      return max_objects;
    }

    std::span<T> Objects() {
      return std::span<T>(std::launder(reinterpret_cast<T*>(memory)), max_objects);
    }

    const std::span<T> Objects() const {
      return std::span<T>(std::launder(reinterpret_cast<T*>(memory)), max_objects);
    }

   private:
    uint8_t* memory = nullptr;
    size_t max_objects = 0;

    void AllocateBlock() {
      FreeBlock();
      memory = (uint8_t*)malloc(max_objects * sizeof(T));
      PROFILE_ALLOCATION(memory, max_objects * sizeof(T));
      memset(memory, 0, max_objects * sizeof(T));
    }

    void FreeBlock() {
      if (memory != nullptr) {
        for (size_t i = 0; i < max_objects; i++) {
          T* ptr = reinterpret_cast<T*>(memory + (i * sizeof(T)));
          std::launder(ptr)->~T();
          memset(ptr, 0, sizeof(T));
        }
      }
      PROFILE_DEALLOCATION(memory);
      free(memory);
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_MEMORY_POOL_HPP