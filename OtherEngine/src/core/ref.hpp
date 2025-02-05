/**
 * \file core/ref.hpp
 */
#ifndef OTHER_ENGINE_REF_HPP
#define OTHER_ENGINE_REF_HPP

#include <concepts>
#include <type_traits>

#include "profiling/profiling.hpp"

#include "core/errors.hpp"
#include "core/ref_counted.hpp"
#include "core/view.hpp"
#include "memory/arena_allocator.hpp"

namespace other {
  namespace detail {

    void RegisterReference(void* instance);
    void RemoveReference(void* instance);
    bool IsValidRef(void* instance);
    size_t NumberOfLivingReferences();

  }  // namespace detail

  template <typename T, typename U>
  concept RefCastable = std::derived_from<U, T>;
  template <typename T>
  concept RefType = std::derived_from<T, RefCounted>;

  template <typename T>
  class Ref {
   public:
    Ref()
        : object(nullptr) {}

    Ref(T* p) {
      object = p;
      IncRef();
    }

    Ref(const Ref<T>& other) {
      object = other.object;
      IncRef();
    }

    Ref(Ref<T>&& other) noexcept {
      object = other.object;
      other.object = nullptr;
    }

    Ref(View<T> view) {
      object = view.object;
      IncRef();
    }

    Ref& operator=(const Ref<T>& other) {
      if (this != &other) {
        object = other.object;
        IncRef();
      }
      return *this;
    }

    Ref& operator=(Ref<T>&& other) noexcept {
      if (this != &other) {
        object = other.object;
        other.object = nullptr;
      }
      return *this;
    }

    template <typename T2>
    Ref(const Ref<T2>& other) {
      object = (T*)other.object;
      IncRef();
    }

    template <typename T2>
    Ref(Ref<T2>&& other) noexcept {
      object = (T*)other.object;
      other.object = nullptr;
    }

    virtual ~Ref() {
      DecRef();
    }

    template <typename T2>
    Ref& operator=(const Ref<T2>& other) {
      static_assert(std::is_base_of_v<T, T2>, "No viable conversion to construct ref with");
      other.IncRef();
      DecRef();

      object = reinterpret_cast<T*>(other.object);
      return *this;
    }

    template <typename T2>
    Ref& operator=(Ref<T2>&& other) noexcept {
      static_assert(std::is_base_of_v<T, T2>, "No viable conversion to construct ref with");
      DecRef();

      object = reinterpret_cast<T*>(other.object);
      other.object = nullptr;

      return *this;
    }

    Ref& operator=(std::nullptr_t) {
      DecRef();
      object = nullptr;
      return *this;
    }

    operator bool() { return object != nullptr; }
    operator bool() const { return object != nullptr; }

    T& operator*() { return *object; }
    const T& operator*() const { return *object; }

    T* operator->() { return object; }
    const T* operator->() const { return object; }

    T* Raw() { return object; }
    const T* Raw() const { return object; }

    void Reset(T* obj = nullptr) {
      if (obj == nullptr) {
        DecRef();
      }
      object = obj;
    }

    template <typename U>
      requires RefCastable<T, U>
    Ref<U> As() const {
      return Ref<U>(*this);
    }

    template <typename U>
      requires RefCastable<T, U>
    static Ref<U> Cast(const Ref<T>& old_ref) {
      return Ref<U>(reinterpret_cast<U*>(old_ref.object));
    }

    template <typename U>
      requires RefCastable<T, U>
    static Ref<T> Clone(const Ref<U>& old_ref) {
      if constexpr (std::same_as<T, U>) {
        return Ref<T>(old_ref);
      } else {
        return Ref<T>(reinterpret_cast<T*>(old_ref.object));
      }

      /// Unreachable
      throw InvalidRefCast(typeid(T), typeid(U));
    }

    template <typename... Args>
    static Ref<T> Create(Args&&... args) {
      return Ref<T>(allocator.Allocate(std::forward<Args>(args)...));
    }

    bool operator==(const Ref<T>& other) const {
      return object == other.object;
    }

    bool operator==(std::nullptr_t) const {
      return object == nullptr;
    }

    bool EqualsObj(const Ref<T>& other) const {
      return object == other.object;
    }

   private:
    static inline ArenaAllocator<T> allocator;
    /// requires mutable to call IncRef and DecRef in const contexts
    mutable T* object;

    /// for direct referncing in cases where we don't want to increment the reference count
    Ref(T* p, bool) {
      object = p;
    }

    void IncRef() const {
      if (object != nullptr) {
        object->Increment();
        detail::RegisterReference(object);
      }
    }

    void DecRef() const {
      if (object != nullptr) {
        object->Decrement();

        if (object->Count() == 0) {
          detail::RemoveReference(object);
          allocator.Free(object);

          object = nullptr;
        }
      }
    }

    template <typename U>
    friend class Ref;
  };

  template <typename T, typename... Args>
    requires RefType<T> && std::constructible_from<T, Args...>
  Ref<T> NewRef(Args&&... args) {
    return Ref<T>::Create(std::forward<Args>(args)...);
  }

}  // namespace other

#endif  // !OTHER_ENGINE_REF_HPP
