/**
 * \file rendering/ui/ui_window_map.cpp
 **/
#include "rendering/ui/ui_window_map.hpp"

namespace other {

  Ref<UIWindow>& UIWindowMap::operator[](UUID id) {
    return windows[id];
  }

  const Ref<UIWindow>& UIWindowMap::at(UUID id) const {
    return windows.at(id);
  }
      
  UIMapItr UIWindowMap::begin() {
    return windows.begin();
  }

  UIMapItr UIWindowMap::end() {
    return windows.end();
  }

  UIMapCItr UIWindowMap::cbegin() const {
    return windows.cbegin();
  }

  UIMapCItr UIWindowMap::cend() const {
    return windows.cend();
  }
      
  UIMapItr UIWindowMap::find(UUID id) {
    return windows.find(id);
  }

  void UIWindowMap::erase(UIMapItr itr) {
    windows.erase(itr);
  }
      
  void UIWindowMap::clear() {
    windows.clear();
  }
      
  void UIWindowMap::AddWindow(UUID id , Ref<UIWindow> window) {
    windows[id] = window;
  }
      
  void UIWindowMap::RemoveWindow(UUID id) {
    auto itr = windows.find(id);
    if (itr == windows.end()) {
      return;
    }

    windows.erase(itr);
  }

} // namespace other
