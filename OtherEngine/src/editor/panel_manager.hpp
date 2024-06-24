/**
 * \file editor/panel_manager.hpp
 **/
#ifndef OTHER_ENGINE_PANEL_MANAGER_HPP
#define OTHER_ENGINE_PANEL_MANAGER_HPP

#include "scene/scene_manager.hpp"

#include "ecs/components/script.hpp"

#include "editor/editor_panel.hpp"

namespace other {

  struct Panel {
    bool panel_open;
    Ref<EditorPanel> panel = nullptr;
  };

  class PanelManager {
    public:
      void Load(Editor* editor , const Ref<Project>& context);
      void Attach(const ConfigTable& editor_config);

      void OnEvent(Event* event);
      void Update(float dt);
      void Render();
      void RenderUI();

      void Detach();
      void Unload();

      void OnSceneLoad(const SceneMetadata* scene_metadata); 
      void OnSceneUnload();

      void OnScriptReload();

    private:
      Ref<Project> project_context = nullptr;
      SceneMetadata* scene_context = nullptr;

      Script editor_scripts;

      std::map<UUID , Panel> active_panels;

      void LoadEditorScripts(const ConfigTable& editor_config);
  };

} // namespace other

#endif // !OTHER_ENGINE_PANEL_MANAGER_HPP
