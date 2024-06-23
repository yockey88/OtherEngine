/**
 * \file editor/editor.hpp
 */
#ifndef OTHER_ENGINE_EDITOR_HPP
#define OTHER_ENGINE_EDITOR_HPP

#include <glad/glad.h>

#include "core/layer.hpp"

#include "parsing/cmd_line_parser.hpp"

#include "project/project.hpp"

#include "application/app.hpp"

#include "ecs/components/script.hpp"

#include "scripting/language_module.hpp"

#include "rendering/camera_base.hpp"

#include "editor/editor_panel.hpp"
#include "editor/scene_panel.hpp"
#include "editor/entity_properties.hpp"

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

      Scope<App> app;
      Ref<Project> project;
      Ref<CameraBase> editor_camera = nullptr;

      Script editor_scripts;

      bool project_panel_open = true;
      Ref<EditorPanel> project_panel;

      bool scene_panel_open = true;
      Ref<ScenePanel> scene_panel;

      bool entity_properties_open = false;
      Ref<EntityProperties> entity_properties_panel;

      Ref<LanguageModule> lua_module = nullptr;

      void SaveActiveScene();
      void LoadEditorScripts();

      virtual void OnLoad() override;
      virtual void OnAttach() override;

      virtual void OnEvent(Event* event) override;
      virtual void Update(float dt) override;
      virtual void Render() override;
      virtual void RenderUI() override;
      virtual void LateUpdate(float dt) override;

      virtual void OnDetach() override;
      virtual void OnUnload() override;

      virtual void OnSceneLoad(const SceneMetadata* path) override;
      virtual void OnSceneUnload() override;

      virtual void OnScriptReload() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_HPP
