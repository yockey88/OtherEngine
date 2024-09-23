/**
 * \file editor/editor.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_LAYER_HPP
#define OTHER_ENGINE_EDITOR_LAYER_HPP

#include "core/layer.hpp"
#include "core/config.hpp"

#include "ecs/components/script.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/scene_renderer.hpp"

#include "editor/panel_manager.hpp"
#include "editor/saves.hpp"

namespace other {

  class EditorLayer : public Layer {
    public:
      EditorLayer(App* editor, const ConfigTable& app_config)
          : Layer(editor , "EditorLayer") , app_config(app_config) {} 
      virtual ~EditorLayer() override {};

      virtual void OnAttach() override;
      virtual void OnDetach() override;
      virtual void OnEarlyUpdate(float dt) override;
      virtual void OnUpdate(float dt) override;
      virtual void OnLateUpdate(float dt) override;
      virtual void OnRender() override;
      virtual void OnUIRender() override;
      virtual void OnEvent(Event* event) override;

      virtual void OnSceneLoad(const SceneMetadata* metadata) override;
      virtual void OnSceneUnload() override;

      virtual void OnScriptReload() override;

    private:
      const ConfigTable& app_config;
      ConfigTable editor_config;

      Opt<StateCapture> saved_scene;

      /// TODO: find a better way to manage state than this
      bool playing = false;
      bool lost_window_focus = false;
      bool camera_free = false;

      Script editor_scripts;

      Ref<CameraBase> editor_camera = nullptr;
      Scope<PanelManager> panel_manager = nullptr;

      Ref<SceneRenderer> default_renderer = nullptr;
      Ref<SceneRenderer> scene_renderer = nullptr;
      Ref<Framebuffer> viewport = nullptr;
      
      /// TODO: move this somewhere else
      void SaveActiveScene();
      void LoadEditorScripts(const ConfigTable& editor_config);

      void LaunchSettingsWindow();

      Ref<SceneRenderer> GetDefaultRenderer();
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_LAYER_HPP
