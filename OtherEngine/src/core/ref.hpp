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
  class Ref : public RefCounted {
    public:
      Ref()
          : object(nullptr) {}

      Ref(std::nullptr_t)
          : object(nullptr) {}

      Ref(T* p) {
        object = p;
        Increment();
      }

      template<typename T2>
      Ref(const Ref<T2>& other) {
      	object = (T*)other.object;
      	IncRef();
      }
      
      template<typename T2>
      Ref(Ref<T2>&& other) {
      	object = (T*)other.object;
      	other.object = nullptr;
      }

      template <typename... Args>
      Ref(Args&&... args) {
        static_assert(std::is_constructible_v<T , Args...> , "Cannot construct a reference from given arguments");
        object = new T(std::forward<Args>(args)...);
        Increment();
      }
      
      virtual ~Ref() override {
        Decrement();
        if (Count() == 0) {
          delete object;
        }
      }

      Ref(const Ref& other) {
        object = other.object;
        Increment();
      }

      /// @note count does not change on move
      Ref(Ref&& other) noexcept {
        object = other.object;
        other.object = nullptr;
      }

      Ref& operator=(const Ref& other) {
        if (this != &other) {
          object = other.object;
          Increment();
        }
        return *this;
      }

      /// @note count does not change on move 
      Ref& operator=(Ref&& other) noexcept {
        if (this != &other) {
          object = other.object;
          other.object = nullptr;
        }
        return *this;
      }

      template<typename T2>
      Ref& operator=(const Ref<T2>& other) {
      	other.IncRef();
      	DecRef();
      
      	object = other.object;
      	return *this;
      }
      
      template<typename T2>
      Ref& operator=(Ref<T2>&& other) {
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
        DecRef();
        this->object = object;
      }

      template <typename U>
      Ref<U> As() const {
        return Ref<U>(*this);
      }

      template <typename... Args>
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
          object->IncRefCount();
          detail::RegisterReference(object); 
        }
      }

      void DecRef() const {
        if (object != nullptr) {
          object->DecRefCount();

          if (object->GetRefCount() == 0) {
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
    return Ref<T>(std::forward<Args>(args)...);
  }

} // namespace other

#endif // !OTHER_ENGINE_REF_HPP
