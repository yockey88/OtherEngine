/**
 * \file editor/panel_manager.hpp
 **/
#ifndef OTHER_ENGINE_PANEL_MANAGER_HPP
#define OTHER_ENGINE_PANEL_MANAGER_HPP

#include "scene/scene_manager.hpp"

#include "editor/editor_panel.hpp"

namespace other {

  struct Panel {
    bool panel_open;
    Ref<EditorPanel> panel = nullptr;
  };

  class PanelManager {
   public:
    void Attach(const Ref<Project>& context, const ConfigTable& editor_config);

    UUID AddPanel(const std::string& name, const Ref<EditorPanel>& panel);
    void RemovePanel(const std::string& name);
    void RemovePanel(const UUID& panel_id);

    void EarlyUpdate(float dt);
    void Update(float dt);
    void LateUpdate(float dt);
    /// placeholder because this matches a generic pattern and might be useful
    void Render();
    bool RenderUI();

    void Detach();

    void OnSceneActivate(const SceneMetadata* scene_metadata);
    void OnSceneDeactivate();

   private:
    Ref<Project> project_context = nullptr;
    SceneMetadata* scene_context = nullptr;

    std::map<UUID, Panel> active_panels;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PANEL_MANAGER_HPP
