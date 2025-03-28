/**
 * \file core/view.hpp
 **/
#ifndef OTHER_ENGINE_VIEW_HPP
#define OTHER_ENGINE_VIEW_HPP

#include <cstddef>

#include "core/ref_counted.hpp"

namespace other {

  template <typename T>
  class Ref;

  template <typename T>
  class View {
   public:
    View() = default;
    View(T& object) { Assign(&object); }
    View(T* object) { Assign(object); }
    View(Ref<T>& ref) { Assign(ref.Raw()); }
    View(const View<T>& other) { Assign(other.object); }
    View(std::nullptr_t) { Assign(nullptr); }

    ~View() { Assign(nullptr); }

    bool operator==(std::nullptr_t) { return object == nullptr; }
    bool operator!=(std::nullptr_t) { return object != nullptr; }

    T* operator->() { return object; }
    const T* operator->() const { return object; }

   private:
    friend class Ref<T>;

    mutable T* object = nullptr;

    void Assign(std::nullptr_t) {
      if (object != nullptr) {
        if constexpr (std::is_base_of_v<RefCounted, T>) {
          object->ViewDecrement();
        }
      }
      object = nullptr;
    }

    void Assign(T* obj) {
      if (object != nullptr) {
        if constexpr (std::is_base_of_v<RefCounted, T>) {
          object->ViewDecrement();
        }
      }
      object = obj;
      if constexpr (std::is_base_of_v<RefCounted, T>) {
        object->ViewIncrement();
      }
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_VIEW_HPP
