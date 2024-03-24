/**
 * \file rendering/ui/ui_window.cpp
 **/
#include "rendering/ui/ui_window.hpp"

#include <imgui/imgui.h>

namespace other {

  void UIWindow::PushRenderFunction(std::function<void()> render_function) {
    render_functions.push_back(render_function);
  }

  void UIWindow::GiveChild(Ref<UIWindow> child) {
    children.push_back(child);
  }

  void UIWindow::Render() {
    if (!ImGui::Begin(title.c_str() , &window_open , flags)) {
      ImGui::End();
      return;
    }

    for (auto& render_function : render_functions) {
      render_function();
    }

    RenderAllChildren();

    ImGui::End();
  }

  void UIWindow::RenderAllChildren() {
    for (auto& child : children) {
      child->Render();
    }
  }

} // namespace other
