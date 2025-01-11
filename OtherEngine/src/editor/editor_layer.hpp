/**
 * \file editor/editor.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_LAYER_HPP
#define OTHER_ENGINE_EDITOR_LAYER_HPP

#include "core/config.hpp"
#include "core/layer.hpp"

#include "event/key_events.hpp"
#include "event/mouse_events.hpp"
#include "event/scene_events.hpp"

#include "scene/scene_capture.hpp"

#include "editor/panel_manager.hpp"

namespace other {

  class EditorLayer : public Layer {
   public:
    EditorLayer(App* editor, const ConfigTable& app_config)
        : Layer(editor, "EditorLayer"), app_config(app_config) {}
    virtual ~EditorLayer() override{};

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEarlyUpdate(float dt) override;
    virtual void OnUpdate(float dt) override;
    virtual void OnLateUpdate(float dt) override;
    virtual void OnRender() override;
    virtual void OnUIRender() override;

   private:
    const ConfigTable& app_config;
    ConfigTable editor_config;

    AssetHandle editor_ray_mesh;

    /// TODO: find a better way to manage state than this
    bool playing = false;
    bool lost_window_focus = false;
    bool rendering_physics_colliders = false;

    Scope<PanelManager> panel_manager = nullptr;
    Ref<Framebuffer> viewport = nullptr;

    void LaunchSettingsWindow();

    Ray CastRay(Ref<CameraBase>& camera, const glm::vec2& mouse_pos);

    bool HandleKeyPressed(KeyPressed& event);
    bool HandleMousePressed(MouseButtonPressed& event);
    bool HandleSceneActivate(SceneActivate& event);
    bool HandleSceneUnload(SceneUnload& event);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EDITOR_LAYER_HPP
