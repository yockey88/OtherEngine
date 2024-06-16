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
#include "editor/editor_panel.hpp"
#include "editor/scene_panel.hpp"
#include "editor/entity_properties.hpp"

#include "scene/scene.hpp"
#include "scripting/language_module.hpp"

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
      Ref<Scene> active_scene;

      bool project_panel_open = true;
      Ref<EditorPanel> project_panel;

      bool scene_panel_open = true;
      Ref<ScenePanel> scene_panel;

      bool entity_properties_open = false;
      Ref<EntityProperties> entity_properties_panel;

      Ref<LanguageModule> lua_module = nullptr;

      virtual void OnAttach() override;
      virtual void OnEvent(Event* event) override;
      virtual void Update(float dt) override;
      virtual void Render() override;
      virtual void RenderUI() override;
      virtual void OnDetach() override;

      virtual void OnSceneLoad(const SceneMetadata* path) override;
      virtual void OnSceneUnload() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_HPP
