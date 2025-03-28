/**
 * \file core/scope.hpp
 **/
#ifndef OTHER_ENGINE_SCOPE_HPP
#define OTHER_ENGINE_SCOPE_HPP

#include <concepts>

#include "memory/arena_allocator.hpp"

namespace other {

  template <typename T>
  class TScope {
   public:
    TScope() : ptr(nullptr) {}
    TScope(T* ptr) : ptr(ptr) {}

    ~TScope() { allocator.Free(ptr); }

    TScope(const TScope&) = delete;
    TScope& operator=(const TScope&) = delete;

    TScope(TScope&& other) noexcept
        : ptr(other.ptr) {
      other.ptr = nullptr;
    }
    TScope& operator=(TScope&& other) noexcept {
      if (this != &other) {
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
      }
      return *this;
    }

    T* operator->() { return ptr; }
    const T* operator->() const { return ptr; }

    T& operator*() { return *ptr; }
    T& operator*() const { return *ptr; }

    bool operator==(std::nullptr_t) const { return ptr == nullptr; }
    bool operator!=(std::nullptr_t) const { return ptr != nullptr; }

    operator bool() const { return ptr != nullptr; }

   private:
    ArenaAllocator<T> allocator;
    mutable T* ptr = nullptr;
  };

  template <typename T, typename... Args>
    requires std::constructible_from<T, Args...>
  TScope<T> NewTScope(Args&&... args) {
    ArenaAllocator<T> allocator;
    return TScope(allocator.Allocate(std::forward<Args>(args)...));
  }

}  // namespace other

#endif  // !OTHER_ENGINE_SCOPE_HPP