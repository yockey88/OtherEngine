/**
 * \file rendering/ui/ui_window.cpp
 **/
#include "rendering/ui/ui_window.hpp"

#include <imgui/imgui.h>

#include "event/event_queue.hpp"
#include "event/ui_events.hpp"

namespace other {

  void UIWindow::PushRenderFunction(std::function<void()> render_function) {
    render_functions.push_back(render_function);
  }

  void UIWindow::GiveChild(Ref<UIWindow> child) {
    children.push_back(child);
  }

  void UIWindow::Render() {
    if (!window_open) {
      return;
    }

    if (!ImGui::Begin(title.c_str() , &window_open , flags)) {
      ImGui::End();
      return;
    }

    for (auto& render_function : render_functions) {
      render_function();
      if (render_function_popped) {
        render_function_popped = false;
        break;
      }
    }
    
    RenderAllChildren();
    
    if (render_functions.size() == 0) {
      window_open = false;
    }

    ImGui::End();
  }

  void UIWindow::Open() { 
    window_open = true; 
  }
  void UIWindow::Close() { 
    window_open = false; 

    EventQueue::PushEvent<UIWindowClosed>(FNV(title));
  }
      
  void UIWindow::PopFrontFunction() {
    render_functions.erase(render_functions.begin());
    render_function_popped = true;
  }

  void UIWindow::RenderAllChildren() {
    for (auto& child : children) {
      child->Render();
    }
  }

} // namespace other
