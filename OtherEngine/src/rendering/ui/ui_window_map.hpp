/**
 * \file rendering/ui/ui_window_map.hpp
 **/
#ifndef OTHER_ENGINE_UI_WINDOW_MAP_HPP
#define OTHER_ENGINE_UI_WINDOW_MAP_HPP

#include <map>

#include "core/uuid.hpp"
#include "rendering/ui/ui_window.hpp"

namespace other {

  using UIMapItr = std::map<UUID , Ref<UIWindow>>::iterator;
  using UIMapCItr = std::map<UUID , Ref<UIWindow>>::const_iterator;

  class UIWindowMap {
    public:
      Ref<UIWindow>& operator[](UUID id);
      const Ref<UIWindow>& at(UUID id) const;

      UIMapItr begin();
      UIMapItr end();

      UIMapCItr cbegin() const;
      UIMapCItr cend() const;

      UIMapItr find(UUID id);
      void erase(UIMapItr itr);
      void clear();

      void AddWindow(UUID id , Ref<UIWindow> window);
      void RemoveWindow(UUID id);

    private:
      std::map<UUID , Ref<UIWindow>> windows;
  };

} // namespace other

#endif // !OTHER_ENGINE_UI_WINDOW_MAP_HPP
