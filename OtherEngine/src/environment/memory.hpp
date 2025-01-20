/**
 * \file environment/memory.hpp
 **/
#ifndef OTHER_ENGINE_MEMORY_HPP
#define OTHER_ENGINE_MEMORY_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

#include "core/defines.hpp"
#include "core/value.hpp"
#include "environment/env_defines.hpp"

namespace other {

  class Memory {
   public:
    Memory() = default;
    ~Memory() = default;

    size_t HeapCursorPos() const;
    size_t StackCursorPos() const;

    Value& ValueWrapperAt(size_t index);
    const Value& ValueWrapperAt(size_t index) const;

    ValueType CheckValueType(address_t addr) const;

    template <typename T>
    address_t Set(size_t idx) {
      if (idx >= kMemorySize) {
        throw std::out_of_range("Attempting to set value on invalid index");
      }

      ValueWrapperAt(idx) = Value(T{});
      return PageFromIndex(idx);
    }

    template <typename T>
    address_t Set(size_t page, size_t page_idx, const T& val) {
      if (page >= kNumPages || page_idx >= kPageSize) {
        throw std::out_of_range("Attempting to set value on invalid page");
      }

      size_t index = page * kPageSize + page_idx;
      if (index >= kMemorySize) {
        throw std::out_of_range("Attempting to set value on invalid index");
      }
      ValueWrapperAt(index) = std::forward<T>(val);

      return { static_cast<uint8_t>(page), static_cast<uint32_t>(page_idx) };
    }

    template <typename T>
      requires(!std::same_as<T, std::string>)
    T& Get(address_t address) {
      size_t index = address.page * kPageSize + address.page_idx;

      if ((address.page >= kNumPages && address.page_idx >= kPageSize) || index >= kMemorySize) {
        throw std::out_of_range("Attempting to get value on invalid page");
      }

      Value& val = ValueWrapperAt(index);
      OE_ASSERT(val.GetType() == GetValueType<T>(), "Attempting to get value of incorrect type");

      T* value = val.GetPtr<T>();
      OE_ASSERT(value != nullptr, "Value is empty!");
      return *value;
    }

    std::string GetString(address_t address) {
      size_t index = address.page * kPageSize + address.page_idx;

      if ((address.page >= kNumPages && address.page_idx >= kPageSize) || index >= kMemorySize) {
        throw std::out_of_range("Attempting to get value on invalid page");
      }

      Value& val = ValueWrapperAt(index);
      OE_ASSERT(val.GetType() == ValueType::STRING, "Attempting to get value of incorrect type");

      return val.Get<std::string>();
    }

    template <typename T>
      requires std::is_trivially_copyable_v<T>
    T* GetPtrTo(address_t address) {
      size_t index = address.page * kPageSize + address.page_idx;
      if ((address.page >= kNumPages && address.page_idx >= kPageSize) || index >= kMemorySize) {
        throw std::out_of_range("Attempting to get value on invalid page");
      }

      Value& val = ValueWrapperAt(index);
      OE_ASSERT(val.GetType() == GetValueType<T>(), "Attempting to get value of incorrect type");

      T* value = val.GetPtr<T>();
      OE_ASSERT(value != nullptr, "Attempting to get value that is nullptr");
      return value;
    }

    template <typename T>
      requires std::default_initializable<T>
    address_t Alloc() {
      if (heap_cursor.position >= kHeapEnd) {
        throw std::out_of_range("Environment memory full, memory bounds incorrect");
      }

      size_t index = heap_cursor.position;
      address_t addr = heap_cursor.address;
      ValueWrapperAt(index) = Value(T{});
      ++heap_cursor;

      return addr;
    }

    template <typename T>
    address_t Alloc(const T& val) {
      if (heap_cursor.position >= kHeapEnd) {
        throw std::out_of_range("Environment memory full, memory bounds incorrect");
      }

      size_t index = heap_cursor.position;
      address_t addr = heap_cursor.address;
      ValueWrapperAt(index) = val;
      ++heap_cursor;

      return addr;
    }

    template <typename T, typename... Args>
      requires std::constructible_from<T, Args...>
    address_t Alloc(Args&&... args) {
      if (heap_cursor.position >= kHeapEnd) {
        throw std::out_of_range("Environment memory full, memory bounds incorrect");
      }

      size_t index = heap_cursor.position;
      address_t addr = heap_cursor.address;

      if constexpr (requires { T{ std::forward<Args>(args)... }; }) {
        ValueWrapperAt(index) = T{ std::forward<Args>(args)... };
      } else {
        T t(std::forward<Args>(args)...);
        ValueWrapperAt(index) = t;
      }

      ++heap_cursor;

      return addr;
    }

    void Free(address_t address);

    template <typename T>
    address_t Push(const T& val) {
      if (stack_cursor.position >= kStackEnd) {
        throw std::out_of_range("Environment memory full, stack overflow detected");
      }

      size_t index = stack_cursor.position;
      address_t addr = stack_cursor.address;
      ValueWrapperAt(index) = val;
      ++stack_cursor;

      return addr;
    }

    template <typename T>
    T Pop() {
      if (stack_cursor.position == kStackStart) {
        throw std::out_of_range("Environment memory empty, stack underflow detected");
      }

      if (GetValueType<T>() != ValueWrapperAt(stack_cursor.position - 1).GetType()) {
        throw std::runtime_error("Attempting to pop value of incorrect type");
      }

      --stack_cursor;
      size_t index = stack_cursor.position;
      return ValueWrapperAt(index).Get<T>();
    }

    constexpr static size_t kPageSize = 1024;
    constexpr static size_t kNumPages = 3;

    constexpr static size_t kMemorySize = kPageSize * kNumPages;

    constexpr static size_t kStackStart = kPageSize * (kNumPages - 1);  /// just the last page
    constexpr static size_t kStackEnd = kMemorySize;
    constexpr static size_t kStackPage = kNumPages - 1;
    constexpr static size_t kStackSize = kStackEnd - kStackStart;

    constexpr static size_t kHeapStart = 0;
    constexpr static size_t kHeapEnd = kPageSize * (kNumPages - 1);  /// just the last page
    constexpr static size_t kHeapSize = kHeapEnd - kHeapStart;

    /// for sanity
    static_assert(kNumPages >= 3 && kNumPages <= 10, "Memory must have at least 3 pages (heap, data, stack) and can not have more than 10 (1 heap, 8 data, 1 stack)");
    static_assert(kMemorySize == (kStackEnd - kStackStart) + (kHeapEnd - kHeapStart), "Memory size must be equal to stack size + heap size");
    static_assert(kStackSize == kPageSize, "Invalid stack size");
    static_assert(kHeapSize == kPageSize * (kNumPages - 1), "Invalid heap size");

   private:
    std::array<Value, kMemorySize> memory;

    struct MemoryCursor {
      size_t position;
      address_t address;

      MemoryCursor() = default;
      MemoryCursor(size_t pos);
      MemoryCursor(uint8_t page, uint32_t page_idx);

      MemoryCursor& operator++();
      MemoryCursor& operator--();
    };

    MemoryCursor heap_cursor = MemoryCursor(kHeapStart);
    MemoryCursor stack_cursor = MemoryCursor(kStackStart);

    MemoryCursor& GetCursorForIndex(size_t index);
    MemoryCursor& GetCursorForPage(uint64_t page);

    address_t PageFromIndex(size_t index) const;

    size_t IndexFromCursor(const MemoryCursor& cursor) const;
    size_t IndexFromPage(uint64_t page, uint64_t page_idx) const;

    MemoryCursor CursorFromIndex(size_t index) const;
    MemoryCursor CursorFromPage(uint64_t page, uint64_t page_idx) const;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_MEMORY_HPP
