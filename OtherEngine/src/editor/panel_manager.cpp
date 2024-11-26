/**
 * \file editor/panel_manager.cpp
 **/
#include "editor/panel_manager.hpp"

#include <array>

#include "editor/panels/entity_properties.hpp"
#include "editor/panels/log_panel.hpp"
#include "editor/panels/project_panel.hpp"
#include "editor/panels/scene_panel.hpp"
#include "editor/panels/viewport_panel.hpp"
#include "editor/selection_manager.hpp"

#include "event/event_handler.hpp"

namespace other {

  constexpr static UUID kProjectPanelId = FNV("ProjectPanel");
  constexpr static UUID kScenePanelId = FNV("ScenePanel");
  constexpr static UUID kPropertiesPanelId = FNV("PropertiesPanel");
  constexpr static UUID kConsolePanelId = FNV("ConsolePanel");
  constexpr static UUID kViewportPanelId = FNV("ViewportPanel");

  constexpr static uint32_t kNumDefaultPanels = 5;

  using PanelBuilder = Ref<EditorPanel> (*)();
  using PanelBuilderPair = std::pair<UUID, PanelBuilder>;

  constexpr static std::array<PanelBuilderPair, kNumDefaultPanels> kPanelBuilderMap{
    PanelBuilderPair{
      kProjectPanelId,
      []() -> Ref<EditorPanel> { return NewRef<ProjectPanel>(); },
    },
    PanelBuilderPair{
      kScenePanelId,
      []() -> Ref<EditorPanel> { return NewRef<ScenePanel>(); },
    },
    PanelBuilderPair{
      kPropertiesPanelId,
      []() -> Ref<EditorPanel> { return NewRef<EntityProperties>(); },
    },
    PanelBuilderPair{
      kConsolePanelId,
      []() -> Ref<EditorPanel> { return NewRef<LogPanel>(); },
    },
    PanelBuilderPair{
      kViewportPanelId,
      []() -> Ref<EditorPanel> { return NewRef<ViewportPanel>(); },
    },
  };

  void PanelManager::Attach(const Ref<Project>& context, const ConfigTable& editor_config) {
    OE_ASSERT(context != nullptr, "Loading editor panel manager with null project context!");

    project_context = context;

    for (const auto& [id, ctor] : kPanelBuilderMap) {
      auto& panel = active_panels[id] = Panel{};
      panel.panel_open = true;
      panel.panel = ctor();
      panel.panel->OnProjectChange(project_context);
      panel.panel->OnAttach();
    }

    OE_DEBUG("Panel Manager attached");
  }

  void PanelManager::EarlyUpdate(float dt) {
    for (auto& [id, panel] : active_panels) {
      if (panel.panel == nullptr) {
        continue;
      }

      panel.panel->OnEarlyUpdate(dt);
    }

    active_panels[kPropertiesPanelId].panel_open = SelectionManager::HasSelection();
  }

  void PanelManager::Update(float dt) {
    for (auto& [id, panel] : active_panels) {
      if (panel.panel == nullptr) {
        continue;
      }

      panel.panel->OnUpdate(dt);
    }

    active_panels[kPropertiesPanelId].panel_open = SelectionManager::HasSelection();
  }

  void PanelManager::LateUpdate(float dt) {
    for (auto& [id, panel] : active_panels) {
      if (panel.panel == nullptr) {
        continue;
      }

      panel.panel->OnLateUpdate(dt);
    }

    active_panels[kPropertiesPanelId].panel_open = SelectionManager::HasSelection();
  }

  void PanelManager::Render() {
  }

  bool PanelManager::RenderUI() {
    bool panel_signal = false;
    for (auto& [id, panel] : active_panels) {
      if (panel.panel == nullptr) {
        continue;
      }

      panel_signal = panel.panel->OnGuiRender(panel.panel_open) || panel_signal;

      if (id == kPropertiesPanelId && SelectionManager::HasSelection() && !panel.panel_open) {
        SelectionManager::ClearSelection();
      }
    }

    return panel_signal;
  }

  void PanelManager::Detach() {
    for (auto& [id, panel] : active_panels) {
      panel.panel_open = false;
      panel.panel = nullptr;
    }
  }

  void PanelManager::OnSceneActivate(const SceneMetadata* scene_metadata) {
    OE_ASSERT(scene_metadata != nullptr, "Attempting to set scene context to null scene in panel manager!");
    OE_ASSERT(scene_metadata->scene != nullptr, "Attempting to set scene context to null scene in panel manager!");

    for (auto& [id, panel] : active_panels) {
      if (panel.panel == nullptr) {
        continue;
      }
      panel.panel->SetSceneContext(scene_metadata->scene);
    }
  }

  void PanelManager::OnSceneDeactivate() {
    for (auto& [id, panel] : active_panels) {
      if (panel.panel == nullptr) {
        continue;
      }
      panel.panel->SetSceneContext(nullptr);
    }
  }

}  // namespace other
