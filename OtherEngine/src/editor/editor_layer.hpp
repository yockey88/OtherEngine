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
      EditorLayer(App* editor)
          : Layer(editor , "EditorLayer") {} 
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
      ConfigTable editor_config;

      StateCapture saved_scene;

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
      Scope<VertexArray> fb_mesh = nullptr;
      Ref<Shader> fbshader = nullptr;
      
      glm::mat4 model1 = glm::mat4(1.f);

      other::AssetHandle model_handle;
      Ref<StaticModel> model = nullptr;
      Ref<ModelSource> model_source = nullptr;
      
      /// TODO: move this somewhere else
      void SaveActiveScene();
      void LoadEditorScripts(const ConfigTable& editor_config);

      void LaunchSettingsWindow();

      Ref<SceneRenderer> GetDefaultRenderer();
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_LAYER_HPP
