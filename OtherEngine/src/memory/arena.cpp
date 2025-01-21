/**
 * \file memory/arena.cpp
 **/
#include "memory/arena.hpp"

#include <stacktrace>

#include "core/logger.hpp"

#include "memory/arena_allocator.hpp"
#include "memory/registers.hpp"
#include "profiling/profiling.hpp"

namespace other {

  Arena* Arena::instance = nullptr;

  Arena* Arena::Instance() {
    OE_ASSERT(instance != nullptr, "Arena instance is null.");
    return instance;
  }

  Registers& Arena::GetRegisters() {
    OE_ASSERT(instance != nullptr, "Arena instance is null.");
    OE_ASSERT(instance->registers != nullptr, "Registers are null.");
    return *instance->registers;
  }

  void Arena::Initialize() {
    PROFILE_SECTION("Arena--Initialize");
    OE_ASSERT(instance == nullptr, "Arena instance is null.");
    instance = (Arena*)malloc(sizeof(Arena));
    PROFILE_ALLOCATION(instance, sizeof(Arena));
    new (instance) Arena();
  }

  void Arena::Shutdown() {
    if (instance != nullptr) {
      instance->~Arena();
    }
    PROFILE_DEALLOCATION(instance);
    free(instance);
    instance = nullptr;
  }

  void* Arena::Allocate(std::size_t size) {
    OE_ASSERT(instance != nullptr, "Arena instance is null.");
    OE_ASSERT(size <= kPageSize, "Allocation size is too large for Arena.");

    PROFILE_SECTION("Arena--Allocate");

    if (instance->page_cursor + size >= kPageSize) {
      instance->AllocatePage();
    }
    OE_ASSERT(instance->page_allocation_cursor < kMaxPages, "Exceeded maximum number of pages.");

    /// FIXME: this alignment offset may not be correct for all cases because the alignment is hardcoded to 16 bytes,
    ///         but I need to figure out how to balance the needed alignment for std containers while keeping 16 byte
    ///         alignment because GPUs read memory in 16 byte chunks
    // clang-format off
    size_t alignment_offset = (instance->page_cursor % kAlignment) != 0 ? 
      kAlignment - (instance->page_cursor % kAlignment) : 0;
    // clang-format on
    instance->page_cursor += alignment_offset;

    void* mem = instance->pages[instance->page_allocation_cursor - 1] + instance->page_cursor;

    instance->total_allocations++;
    instance->allocated_memory += size;
    instance->page_cursor += size;

#ifdef OTHER_DEBUG_BUILD
    instance->ReportAllocation(mem, size);
#endif

    return mem;
  }

  void Arena::Free(void* ptr, std::size_t size) {
    /// do nothing for now, allocators handle calling destructors and zeroing memory
    ///   later we can implement a free list or something or register freed chunks for defragmentation
    if (ptr == nullptr) {
      OE_CRITICAL("Attempted to free null pointer.");
      println("{}", std::stacktrace::current());
      return;
    }
#ifdef OTHER_DEBUG_BUILD
    instance->ReportDeallocation(ptr, size);
#endif
    return;
  }

  void Arena::SetArenaInstance(Arena* arena) {
    instance = arena;
  }

  Arena::Arena() {
    page_cursor = 0;
    total_allocations = 0;
    allocated_memory = 0;
    page_allocation_cursor = 0;

    for (size_t i = 0; i < kMaxPages; i++) {
      pages[i] = nullptr;
    }
    pages[page_allocation_cursor] = (uint8_t*)malloc(kPageSize);
    PROFILE_ALLOCATION(pages[page_allocation_cursor], kPageSize);
    page_allocation_cursor++;
    page_cursor = 0;

    OE_ASSERT(pages[0] != nullptr, "Failed to allocate page.");

    ArenaAllocator<Registers> alloc;
    registers = alloc.Allocate();
  }

  Arena::~Arena() {
    ArenaAllocator<Registers> alloc;
    alloc.Free(registers);

    for (size_t i = 0; i < page_allocation_cursor; i++) {
      PROFILE_DEALLOCATION(pages[i]);
      free(pages[i]);
      pages[i] = nullptr;
    }
    page_allocation_cursor = 0;
    page_cursor = 0;
  }

  void Arena::AllocatePage() {
    OE_TRACE("Attempting to page allocation.");
    OE_ASSERT(page_allocation_cursor < kMaxPages, "Exceeded maximum number of pages. Allocating page : {}.", page_allocation_cursor);
    OE_ASSERT(pages[page_allocation_cursor] == nullptr, "Page already allocated.");

    PROFILE_SECTION("Arena--AllocatePage");

    pages[page_allocation_cursor] = (uint8_t*)malloc(kPageSize);
    OE_ASSERT(pages[page_allocation_cursor] != nullptr, "Failed to allocate page.");

    page_allocation_cursor++;
    page_cursor = 0;
  }

#ifdef OTHER_DEBUG_BUILD
  void Arena::ReportAllocation(void* addr, std::size_t sz) {
    std::stringstream ss;
    ss << "Allocated memory at address: " << addr << ", size: " << sz << "\n";
    ss << "   > Total Allocations: " << total_allocations << "\n";
    ss << "   > Allocated Memory: " << allocated_memory << "\n";
    ss << "   > Page Allocation Cursor: " << page_allocation_cursor << "\n";
    ss << "   > Page Cursor: " << page_cursor << "\n";
    println(ss.str());

    auto [it, inserted] = allocations.insert({ addr, sz });
    if (!inserted) {
      OE_ERROR("Failed to insert allocation into map. Memory address [{:p}] in use", addr);
    }
  }

  void Arena::ReportDeallocation(void* ptr, std::size_t sz) {
    std::stringstream ss;
    ss << "Deallocated memory at address: " << ptr << ", size: " << sz << "\n";
    ss << "   > Total Allocations: " << total_allocations << "\n";
    ss << "   > Allocated Memory: " << allocated_memory << "\n";
    ss << "   > Page Allocation Cursor: " << page_allocation_cursor << "\n";
    ss << "   > Page Cursor: " << page_cursor << "\n";
    println(ss.str());

    auto it = allocations.find(ptr);
    OE_ASSERT(it != allocations.end(), "Failed to find deallocated memory address in map.");
    allocations.erase(it);
  }
#endif

}  // namespace other