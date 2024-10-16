/**
 * \file rendering/ui/ui_window.hpp
 **/
#ifndef OTHER_ENGINE_UI_WINDOW_HPP
#define OTHER_ENGINE_UI_WINDOW_HPP

#include <string>
#include <vector>
#include <functional>

#include <imgui/imgui.h>

#include "core/ref_counted.hpp"
#include "core/ref.hpp"
#include "event/event.hpp"

namespace other {

#define UI_FUNC(name) \
  std::bind_front((&name) , this)

#define BIND_FUNC(name , obj) \
  std::bind_front((&name) , obj)

  class UIWindow : public RefCounted {
    public:
      UIWindow(const std::string& title , ImGuiWindowFlags flags = 0 , bool open = true , bool pinned = false) 
        : id(FNV(title)) , title(title) , window_open(open) , pinned(pinned) , flags(flags) {}
      virtual ~UIWindow() {}

      void PushRenderFunction(std::function<void()> render_function);

      void GiveChild(Ref<UIWindow> child);

      virtual void OnAttach() {}
      virtual void OnDetach() {}
      virtual void OnUpdate(float dt) {}
      virtual void OnEvent(Event* event) {}
      virtual void Render();

      const std::string& Title() const { return title; }
      inline bool IsOpen() const { return window_open; }
      inline bool Pinned() const { return pinned; }
      void Open(); 
      void Close();
      inline void Pin() { pinned = true; }

      inline uint64_t ID() const { return id; }

    protected:
      uint64_t id;
      std::string title;
      std::vector<std::function<void()>> render_functions;
      std::vector<Ref<UIWindow>> children;

      bool window_open;
      bool pinned;

      bool render_function_popped = false;

      ImGuiWindowFlags flags;

      void PopFrontFunction();
      void RenderAllChildren();
  };

} // namespace other

template <typename T>
concept ui_type = std::derived_from<T , other::UIWindow>;

#endif // !OTHER_ENGINE_UI_WINDOW_HPP
