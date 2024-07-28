/**
 * \file editor/panel_manager.cpp
 **/
#include "editor/panel_manager.hpp"

#include <array>

#include "event/event_handler.hpp"

#include "editor/project_panel.hpp"
#include "editor/scene_panel.hpp"
#include "editor/entity_properties.hpp"
#include "editor/console_panel.hpp"
#include "editor/selection_manager.hpp"

namespace other {

  constexpr static UUID kProjectPanelId = FNV("ProjectPanel");
  constexpr static UUID kScenePanelId = FNV("ScenePanel");
  constexpr static UUID kPropertiesPanelId = FNV("PropertiesPanel");
  constexpr static UUID kConsolePanelId = FNV("ConsolePanel");

  constexpr static uint32_t kNumDefaultPanels = 4;

  using PanelBuilder = Ref<EditorPanel>(*)(Editor&);
  using PanelBuilderPair = std::pair<UUID , PanelBuilder>;

  constexpr static std::array<PanelBuilderPair , kNumDefaultPanels> kPanelBuilderMap {
    PanelBuilderPair{ kProjectPanelId    , [](Editor& editor) -> Ref<EditorPanel> { return NewRef<ProjectPanel>(editor); } } ,
    PanelBuilderPair{ kScenePanelId      , [](Editor& editor) -> Ref<EditorPanel> { return NewRef<ScenePanel>(editor);  } } ,
    PanelBuilderPair{ kPropertiesPanelId , [](Editor& editor) -> Ref<EditorPanel> { return NewRef<EntityProperties>(editor); } } ,
    PanelBuilderPair{ kConsolePanelId    , [](Editor& editor) -> Ref<EditorPanel> { return NewRef<ConsolePanel>(editor); } } ,
  };

  void PanelManager::Attach(Editor* editor , const Ref<Project>& context , const ConfigTable& editor_config) {
    OE_ASSERT(editor != nullptr , "Loading editor panel manager with a null editor!");
    OE_ASSERT(context != nullptr , "Loading editor panel manager with null project context!");

    project_context = context;

    for (const auto& [id , ctor] : kPanelBuilderMap) {
      auto& panel = active_panels[id] = Panel{};
      panel.panel_open = true;
      panel.panel = ctor(*editor);
      panel.panel->OnProjectChange(project_context);
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
    // for (auto& [id , panel] : active_panels) {
    //   panel.panel->OnUpdate(dt);
    //   // panel.panel_open = panel.panel->IsOpen();
    // }

    active_panels[kPropertiesPanelId].panel_open = SelectionManager::HasSelection();
  }
  
  void PanelManager::Render() {
  }

  bool PanelManager::RenderUI() {
    bool panel_signal = false;
    for (auto& [id , panel] : active_panels) {
      panel_signal = panel.panel->OnGuiRender(panel.panel_open) || panel_signal;

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
