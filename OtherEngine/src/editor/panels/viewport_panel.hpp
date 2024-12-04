/**
 * \file editor/panels/viewport_panel.hpp
 **/
#ifndef OTHER_ENGINE_VIEWPORT_PANEL_HPP
#define OTHER_ENGINE_VIEWPORT_PANEL_HPP

#include "core/ref.hpp"

#include "scene/scene.hpp"

#include "editor/editor_panel.hpp"

namespace other {

  class ViewportPanel : public EditorPanel {
   public:
    ViewportPanel() {}
    virtual ~ViewportPanel() override {}

    virtual void OnRender() override;
    virtual bool OnGuiRender(bool& is_open) override;
    virtual void OnProjectChange(const Ref<Project>& project) override;
    virtual void SetSceneContext(const Ref<Scene>& scene) override;

   private:
    Ref<Scene> active_scene;

    glm::vec2 viewport_padding = { 0.f, 0.f };

    void RenderViewport(const Ref<Framebuffer>& viewport);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_VIEWPORT_PANEL_HPP
