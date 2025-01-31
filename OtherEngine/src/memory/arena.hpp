/**
 * \file memory/arena.hpp
 **/
#ifndef OTHERE_ENGINE_ARENA_HPP
#define OTHERE_ENGINE_ARENA_HPP

#include <cstdint>
#include <map>
#ifndef OTHER_WINDOWS
  #include <cstddef>
#endif

namespace other {

  class Registers;

  /**
   * TODO:
   *       - implement a defragmentation algorithm to run at engine state changes that would reduce fragmentation
   *       - implemenet smarter paging to reduce fragmentation of memory and group allocated objects that are often accessed together or at similar times
   *       - implement memory re-use to reduce the number of page allocations
   **/

  class Arena {
   public:
    static inline constexpr size_t kPageSize = 64 * 4096u * 4096u;
    /// TODO: this feels wrong, this is correct for the GPU but seems incorrect if
    ///         aiming to be as cross platform as possible (research: confirm all GPUs read mem in 16 byte chunks)
    static inline constexpr size_t kAlignment = 16;
    static inline constexpr size_t kMaxPages = 16;
    static inline constexpr size_t kMaxMemoryAllowed = Arena::kMaxPages * Arena::kPageSize;

    struct Page {
      size_t cursor = 0;
      uint8_t data[kPageSize];
    };

    static Arena* Instance();
    static Registers& GetRegisters();

    static void Initialize();
    static void Shutdown();

    static void* Allocate(size_t size);
    static void Free(void* ptr, size_t size);

   private:
    friend class ModuleRegistry;

    static void SetArenaInstance(Arena* arena);

    Arena();
    ~Arena();

    Arena(Arena&&) = delete;
    Arena(const Arena&) = delete;
    Arena& operator=(Arena&&) = delete;
    Arena& operator=(const Arena&) = delete;

    static Arena* instance;

    Registers* registers = nullptr;

    size_t page_allocation_cursor = 0;
    size_t page_cursor = 0;
    size_t total_allocations = 0;
    size_t allocated_memory = 0;

    uint8_t* pages[kMaxPages];

    void AllocatePage();

#ifdef OTHER_MEMORY_DEBUG_BUILD
    std::map<void*, size_t> allocations;
    void ReportAllocation(void* ptr, std::size_t size);
    void ReportDeallocation(void* ptr, std::size_t size);
#endif
  };

}  // namespace other

#endif  // !OTHERE_ENGINE_ARENA_HPP
