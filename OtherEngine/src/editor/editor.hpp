/**
 * \file editor/editor.hpp
 */
#ifndef OTHER_ENGINE_EDITOR_HPP
#define OTHER_ENGINE_EDITOR_HPP

#include <glad/glad.h>

#include "core/layer.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "application/app.hpp"

#include "rendering/camera_base.hpp"
#include "ecs/components/script.hpp"

#include "editor/panel_manager.hpp"

namespace other {

  class Editor : public App {
    public:
      Editor(Engine* engine , Scope<App>& app);
      virtual ~Editor() override {} 

    private:
      const CmdLine& cmdline;
      const ConfigTable& engine_config;
      ConfigTable editor_config;

      /// TODO: find a better way to manage state than this
      bool playing = false;

      Script editor_scripts;

      Scope<App> app;

      Ref<CameraBase> editor_camera = nullptr;

      Scope<PanelManager> panel_manager = nullptr;

      void SaveActiveScene();

      virtual void OnLoad() override;
      virtual void OnAttach() override;

      virtual void OnEvent(Event* event) override;
      virtual void EarlyUpdate(float dt) override;
      virtual void Update(float dt) override;
      virtual void Render() override;
      virtual void RenderUI() override;
      virtual void LateUpdate(float dt) override;

      virtual void OnDetach() override;
      virtual void OnUnload() override;

      virtual void OnSceneLoad(const SceneMetadata* path) override;
      virtual void OnSceneUnload() override;

      virtual void OnScriptReload() override;
      
      void LoadEditorScripts(const ConfigTable& editor_config);
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_HPP
