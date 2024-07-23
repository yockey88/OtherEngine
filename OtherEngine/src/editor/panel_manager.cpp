/**
 * \file editor/panel_manager.cpp
 **/
#include "editor/panel_manager.hpp"

#include "event/event_handler.hpp"

#include "editor/project_panel.hpp"
#include "editor/scene_panel.hpp"
#include "editor/entity_properties.hpp"
#include "editor/selection_manager.hpp"

namespace other {

  constexpr UUID kProjectPanelId = FNV("ProjectPanel");
  constexpr UUID kScenePanelId = FNV("ScenePanel");
  constexpr UUID kPropertiesPanelId = FNV("PropertiesPanel");

  void PanelManager::Attach(Editor* editor , const Ref<Project>& context , const ConfigTable& editor_config) {
    OE_ASSERT(editor != nullptr , "Loading editor panel manager with a null editor!");
    OE_ASSERT(context != nullptr , "Loading editor panel manager with null project context!");

    project_context = context;

    auto& proj_panel = active_panels[kProjectPanelId] = Panel{};
    auto& scene_panel = active_panels[kScenePanelId] = Panel{};
    auto& props_panel = active_panels[kPropertiesPanelId] = Panel{};

    proj_panel.panel_open = true;
    proj_panel.panel = Ref<ProjectPanel>::Create(*editor); 
    proj_panel.panel->OnProjectChange(project_context);

    scene_panel.panel_open = true;
    scene_panel.panel = Ref<ScenePanel>::Create(*editor); 

    props_panel.panel_open = true;
    props_panel.panel = Ref<EntityProperties>::Create(*editor); 
    for (auto& [id , panel] : active_panels) {
      panel.panel->OnAttach();
    }

    OE_DEBUG("Panel Manager attached");
  }

  void PanelManager::OnEvent(Event* event) {
    EventHandler handler(event);

    for (auto& [id , panel] : active_panels) {
      panel.panel->OnEvent(event);      
    } 
  }
  
  void PanelManager::Update(float dt) {
    active_panels[kPropertiesPanelId].panel_open = SelectionManager::HasSelection();
  }
  
  void PanelManager::Render() {
  }

  bool PanelManager::RenderUI() {
    bool panel_signal = false;
    for (auto& [id , panel] : active_panels) {
      panel_signal = panel.panel->OnGuiRender(panel.panel_open);

      if (id == kPropertiesPanelId && SelectionManager::HasSelection() && 
          !panel.panel_open) {
        SelectionManager::ClearSelection();
      }
    }

    return panel_signal;
  }

  void PanelManager::Detach() {
    for (auto& [id , panel] : active_panels) {
      panel.panel_open = false;
      panel.panel = nullptr;
    }
  }

  void PanelManager::OnSceneLoad(const SceneMetadata* scene_metadata) {
    OE_ASSERT(scene_metadata != nullptr , "Attempting to set scene context to null scene in panel manager!");

    for (auto& [id , panel] : active_panels) {
      panel.panel->SetSceneContext(scene_metadata->scene);
    }
  }

  void PanelManager::OnSceneUnload() {
    for (auto& [id , panel] : active_panels) {
      panel.panel->SetSceneContext(nullptr);
    }
  }

  void PanelManager::OnScriptReload() {
    for (auto& [id , panel] : active_panels) {
      panel.panel->OnScriptReload();
    }
  }

} // namespace other
