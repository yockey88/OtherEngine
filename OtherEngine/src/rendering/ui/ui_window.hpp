/**
 * \file rendering/ui/ui_window.hpp
 **/
#ifndef OTHER_ENGINE_UI_WINDOW_HPP
#define OTHER_ENGINE_UI_WINDOW_HPP

#include <string>
#include <vector>
#include <functional>

#include <imgui/imgui.h>

#include "core/ref.hpp"
#include "event/event.hpp"

namespace other {

  class UIWindow {
    public:
      UIWindow(const std::string& title , ImGuiWindowFlags flags = 0 , bool open = true , bool pinned = false) 
        : id(FNV(title)) , title(title) , window_open(open) , pinned(pinned) , flags(flags) {}
      virtual ~UIWindow() {}

      void PushRenderFunction(std::function<void()> render_function);

      void GiveChild(Ref<UIWindow> child);

      virtual void OnAttach() {}
      virtual void OnDetach() {}
      virtual void OnUpdate(float dt) {}
      virtual void OnEvent(Event& event) {}
      virtual void Render();

      const std::string& Title() const { return title; }
      inline bool IsOpen() const { return window_open; }
      inline bool Pinned() const { return pinned; }
      inline void Open() { window_open = true; }
      inline void Close() { window_open = false; }
      inline void Pin() { pinned = true; }

      inline uint64_t ID() const { return id; }

    protected:
      uint64_t id;
      std::string title;
      std::vector<std::function<void()>> render_functions;
      std::vector<Ref<UIWindow>> children;

      bool window_open;
      bool pinned;

      ImGuiWindowFlags flags;

      void RenderAllChildren();
  };

} // namespace other

template <typename T>
concept ui_type = std::derived_from<T , other::UIWindow>;

#endif // !OTHER_ENGINE_UI_WINDOW_HPP
