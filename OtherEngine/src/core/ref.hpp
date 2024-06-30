/**
 * \file core/ref.hpp
*/
#ifndef OTHER_ENGINE_REF_HPP
#define OTHER_ENGINE_REF_HPP

#include "core/ref_counted.hpp"

namespace other {
namespace detail {

  void RegisterReference(void* instance);
  void RemoveReference(void* instance);
  bool IsValidRef(void* instance);

} // namespace detail

  template <typename T>
  class Ref {
    public:
      Ref()
          : object(nullptr) {}

      Ref(std::nullptr_t)
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
      
      	object = other.object;
      	return *this;
      }
      
      template<typename T2>
      Ref& operator=(Ref<T2>&& other) noexcept {
        static_assert(std::is_base_of_v<T , T2> , "No viable conversion to construct ref with");
      	DecRef();
      
      	object = other.object;
      	other.object = nullptr;
      	return *this;
      }

      operator bool() { return object != nullptr; }
      operator bool() const { return object != nullptr; }
      
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
      Ref<U> As() const {
        return Ref<U>(*this);
      }

      template <typename U>
      static Ref<U> Cast(Ref<T>& old_ref) {
        static_assert(std::is_base_of_v<RefCounted , U> , "Cannot cast a reference to a non-refcounted object");
        return Ref<U>((U*)old_ref.object);
      }

      static Ref<T> Clone(const Ref<T>& old_ref) {
        return Ref<T>(old_ref);
      }

      template <typename... Args>
      static Ref<T> Create(Args&&... args) {
        static_assert(std::is_constructible_v<T , Args...> , "Cannot construct a reference from given arguments");
        static_assert(std::is_base_of_v<RefCounted , T> , "Cannot create a reference to a non-refcounted object");
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
  Ref<T> NewRef(Args&&... args) {
    static_assert(std::is_constructible_v<T , Args...> , "Cannot construct a reference from given arguments");
    static_assert(std::is_base_of_v<RefCounted , T> , "Cannot create a reference to a non-refcounted object");
    return Ref<T>::Create(std::forward<Args>(args)...);
  }

} // namespace other

#endif // !OTHER_ENGINE_REF_HPP
