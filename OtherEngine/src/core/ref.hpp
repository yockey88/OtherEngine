/**
 * \file core/ref.hpp
*/
#ifndef OTHER_ENGINE_REF_HPP
#define OTHER_ENGINE_REF_HPP

#include <concepts>
#include <type_traits>

#include "core/errors.hpp"
#include "core/ref_counted.hpp"

namespace other {
namespace detail {

  void RegisterReference(void* instance);
  void RemoveReference(void* instance);
  bool IsValidRef(void* instance);

} // namespace detail

  template <typename T , typename U>
  concept RefCastable = std::is_convertible_v<T, U> || std::is_base_of_v<T, U>;

    // requires std::is_base_of_v<RefCounted , T>
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

      template<typename T2>
      Ref(const Ref<T2>& other) {
      	object = (T*)other.object;
      	IncRef();
      }
      
      template<typename T2>
      Ref(Ref<T2>&& other) noexcept {
      	object = (T*)other.object;
      	other.object = nullptr;
      }
      
      virtual ~Ref() {
        DecRef();
      }

      template<typename T2>
      Ref& operator=(const Ref<T2>& other) {
        static_assert(std::is_base_of_v<T , T2> , "No viable conversion to construct ref with");
      	other.IncRef();
      	DecRef();
      
      	object = reinterpret_cast<T*>(other.object);
      	return *this;
      }
      
      template<typename T2>
      Ref& operator=(Ref<T2>&& other) noexcept {
        static_assert(std::is_base_of_v<T , T2> , "No viable conversion to construct ref with");
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

      Ref& operator*() { return object; }
      
      T* operator->() { return object; }
      const T* operator->() const { return object; }

      T* Raw() { return object; }
      const T* Raw() const { return object; }

      void Reset(T* object = nullptr) {
        this->object = object;
        if (this->object == nullptr) {
          DecRef();
        }
      }

      template <typename U>
        requires RefCastable<T , U>
      Ref<U> As() const {
        return Ref<U>(*this);
      }

      template <typename U>
        requires RefCastable<T , U>
      static Ref<U> Cast(Ref<T>& old_ref) {
        return Ref<U>(reinterpret_cast<U*>(old_ref.object));
      }

      template <typename U>
        requires RefCastable<T , U>
      static Ref<T> Clone(const Ref<U>& old_ref) {
        if constexpr (std::same_as<T , U>) {
          return Ref<T>(old_ref);
        } else {
          return Ref<T>(reinterpret_cast<T*>(old_ref.object));
        }

        /// Unreachable
        throw InvalidRefCast(typeid(T) , typeid(U));
      }

      template <typename... Args>
        requires requires (Args&&... args) {
          requires std::is_base_of_v<RefCounted , T>;
          requires std::is_constructible_v<T , Args...>;
        }
      static Ref<T> Create(Args&&... args) {
        return Ref<T>(new T(std::forward<Args>(args)...));
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
      mutable T* object;

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
            delete object;
            object = nullptr;
          }
        }
      }

      template <typename U>
      friend class Ref;
  };

  template <typename T , typename... Args>
    requires requires (Args&&... args) {
      requires std::is_base_of_v<RefCounted , T>;
      requires std::is_constructible_v<T , Args...>;
    }
  Ref<T> NewRef(Args&&... args) {
    return Ref<T>::Create(std::forward<Args>(args)...);
  }

} // namespace other

#endif // !OTHER_ENGINE_REF_HPP
