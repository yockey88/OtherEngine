/**
 * \file editor/panels/viewport_panel.hpp
 **/
#ifndef OTHER_ENGINE_VIEWPORT_PANEL_HPP
#define OTHER_ENGINE_VIEWPORT_PANEL_HPP

#include "core/ref.hpp"

#include "event/key_events.hpp"

#include "scene/scene.hpp"

#include "editor/editor_panel.hpp"

namespace other {

  class ViewportPanel : public EditorPanel {
   public:
    ViewportPanel() {}
    virtual ~ViewportPanel() override {}

    virtual void OnAttach() override;

    virtual void OnRender() override;
    virtual bool OnGuiRender(bool& is_open) override;
    virtual void OnProjectChange(const Ref<Project>& project) override;
    virtual void SetSceneContext(const Ref<Scene>& scene) override;

   private:
    Ref<Scene> active_scene;
    glm::vec2 viewport_padding = { 0.f, 0.f };

    Ref<Framebuffer> viewport;

    bool window_hovered = false;
    bool open_framebuffer_menu = false;

    void RenderFramebufferList(Ref<SceneRenderer>& renderer);
    void RenderViewport(const Ref<Framebuffer>& viewport);

    bool HandleKeyPressed(KeyPressed& event);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_VIEWPORT_PANEL_HPP
