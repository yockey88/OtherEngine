/**
 * \file editor/editor.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_LAYER_HPP
#define OTHER_ENGINE_EDITOR_LAYER_HPP

#include "core/config.hpp"
#include "core/layer.hpp"

#include "event/key_events.hpp"

#include "ecs/components/script.hpp"

#include "rendering/camera_base.hpp"

#include "editor/panel_manager.hpp"
#include "editor/saves.hpp"

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

    Opt<StateCapture> saved_scene;

    /// TODO: find a better way to manage state than this
    bool playing = false;
    bool lost_window_focus = false;
    bool camera_free = false;

    Script editor_scripts;

    Scope<PanelManager> panel_manager = nullptr;
    Ref<CameraBase> editor_camera = nullptr;

    glm::vec2 current_viewport_size = { 0.f, 0.f };
    Ref<Framebuffer> viewport = nullptr;

    /// TODO: move this somewhere else
    void SaveActiveScene();
    void LoadEditorScripts(const ConfigTable& editor_config);

    void LaunchSettingsWindow();

    bool HandleKeyPressed(KeyPressed& event);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EDITOR_LAYER_HPP
