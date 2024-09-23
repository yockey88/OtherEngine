/**
 * \file editor_layer.hpp
 **/
#ifndef OTHER_ENGINE_TEST_EDITOR_LAYER_HPP
#define OTHER_ENGINE_TEST_EDITOR_LAYER_HPP

#include "core/config.hpp"
#include "core/layer.hpp"

#include "ecs/components/script.hpp"

namespace other {

  class TEditorLayer : public Layer {
    public:
      TEditorLayer(App* editor, const ConfigTable& app_config) 
          : Layer(editor , "TEditorLayer") , app_config(app_config) {}
      virtual ~TEditorLayer() override {}

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

      Ref<CameraBase> editor_camera = nullptr;
      Ref<SceneRenderer> scene_renderer = nullptr;

      Script editor_scripts;

      void LoadScripts();
      Ref<SceneRenderer> CreateRenderer();
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_LAYER_HPP
