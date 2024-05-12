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
#include "rendering/framebuffer.hpp"
#include "editor/editor_panel.hpp"
#include "editor/scene_panel.hpp"
#include "editor/entity_properties.hpp"

#include "scene/scene.hpp"

namespace other {

  class Editor : public App {
    public:
      Editor(Engine* engine , Scope<App>& app);
      ~Editor() = default;

      void LoadScene(const Path& path);

    private:
      const CmdLine& cmdline;
      const ConfigTable& engine_config;
      ConfigTable editor_config;

      Scope<App> app;
      Ref<Project> project;
      Ref<Scene> active_scene;

      bool project_panel_open = true;
      Ref<EditorPanel> project_panel;

      bool scene_panel_open = true;
      Ref<ScenePanel> scene_panel;

      bool entity_properties_open = false;
      Ref<EntityProperties> entity_properties_panel;

      Scope<Framebuffer> viewport = nullptr;

      uint32_t fbo = 0;
      uint32_t rbo = 0;
      uint32_t framebuffer = 0;

      uint32_t intermediate_fbo = 0;
      uint32_t screen_texture = 0;

      uint32_t color_attachment = 0;

      bool fb_complete = false;

      virtual void OnAttach() override;
      virtual void OnEvent(Event* event) override;
      virtual void Update(float dt) override;
      virtual void Render() override;
      virtual void RenderUI() override;
      virtual void OnDetach() override;

      virtual void OnSceneLoad(const Path& path) override;
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_HPP
