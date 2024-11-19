/**
 * \file environment/memory.cpp
 **/
#include "environment/memory.hpp"

namespace other {

  size_t Memory::HeapCursorPos() const {
    return heap_cursor.position;
  }

  size_t Memory::StackCursorPos() const {
    return stack_cursor.position;
  }

  Value& Memory::ValueWrapperAt(size_t index) {
    OE_ASSERT(index < kMemorySize, "Attempting to get value on invalid index");
    return memory.at(index);
  }

  const Value& Memory::ValueWrapperAt(size_t index) const {
    OE_ASSERT(index < kMemorySize, "Attempting to get value on invalid index");
    return memory.at(index);
  }
    
  ValueType Memory::CheckValueType(address_t addr) const {
    OE_ASSERT(addr.page < kNumPages && addr.page_idx < kPageSize , "Invalid address!");

    size_t idx = addr.page * kPageSize + addr.page_idx;
    OE_ASSERT(idx < kMemorySize, "Out of bounds error!");

    const Value& v = ValueWrapperAt(idx);
    if (v.Empty()) {
      return ValueType::EMPTY_TYPE;
    }
    return v.Type();
  }

  Memory::MemoryCursor::MemoryCursor(size_t pos) {
    position = pos;
    address = { uint8_t(pos / kPageSize), uint32_t(pos % kPageSize) };
  }

  Memory::MemoryCursor::MemoryCursor(uint8_t page, uint32_t page_idx) {
    address = { page, page_idx };
  }

  Memory::MemoryCursor& Memory::MemoryCursor::operator++() {
    position++;
    address = { uint8_t(position / kPageSize), uint32_t(position % kPageSize) };
    return *this;
  }

  Memory::MemoryCursor& Memory::MemoryCursor::operator--() {
    position--;
    address = { uint8_t(position / kPageSize), uint32_t(position % kPageSize) };
    return *this;
  }

  Memory::MemoryCursor& Memory::GetCursorForIndex(size_t index) {
    if (index >= kHeapEnd) {
      return stack_cursor;
    }

    return heap_cursor;
  }

  Memory::MemoryCursor& Memory::GetCursorForPage(uint64_t page) {
    if (page == kNumPages - 1) {
      return stack_cursor;
    }

    return heap_cursor;
  }

  address_t Memory::PageFromIndex(size_t index) const {
    return MemoryCursor(index).address;
  }

  size_t Memory::IndexFromCursor(const MemoryCursor& cursor) const {
    return cursor.position;
  }

  size_t Memory::IndexFromPage(uint64_t page, uint64_t page_idx) const {
    return page * kPageSize + page_idx;
  }

  Memory::MemoryCursor Memory::CursorFromIndex(size_t index) const {
    return MemoryCursor(index);
  }

  Memory::MemoryCursor Memory::CursorFromPage(uint64_t page, uint64_t page_idx) const {
    return MemoryCursor(page * kPageSize + page_idx);
  }

  void Memory::Free(address_t address) {
    size_t index = address.page * kPageSize + address.page_idx;
    if (index >= kMemorySize) {
      throw std::out_of_range("Attempting to free value on invalid index");
    }

    ValueWrapperAt(index) = Value();
  }

}  // namespace other
