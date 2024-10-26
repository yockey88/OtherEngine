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

  void UIWindowMap::AddWindow(UUID id, Ref<UIWindow> window) {
    OE_ASSERT(window != nullptr, "Can not add null window to UIWindowMap");
    auto itr = windows.find(id);
    if (itr != windows.end()) {
      OE_WARN("UIWindow with id: {} already exists, overwriting", id);
      return;
    }

    window->OnAttach();
    windows[id] = window;
  }

  void UIWindowMap::RemoveWindow(UUID id) {
    OE_ASSERT(windows.find(id) != windows.end(), "UIWindow with id: {} does not exist", id);

    auto itr = windows.find(id);
    if (itr == windows.end()) {
      OE_WARN("UIWindow with id: {} does not exist", id);
      return;
    }

    windows[id]->OnDetach();
    windows.erase(itr);
  }

}  // namespace other
