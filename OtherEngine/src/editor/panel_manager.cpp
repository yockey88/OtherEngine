/**
 * \file editor/panel_manager.cpp
 **/
#include "editor/panel_manager.hpp"

#include <array>

#include "core/uuid.hpp"

#include "event/event_handler.hpp"

#include "editor/panels/entity_properties.hpp"
#include "editor/panels/log_panel.hpp"
#include "editor/panels/project_panel.hpp"
#include "editor/panels/scene_panel.hpp"
#include "editor/panels/viewport_panel.hpp"
#include "editor/selection_manager.hpp"

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

  UUID PanelManager::AddPanel(const std::string& name, const Ref<EditorPanel>& panel) {
    UUID id = FNV(name);
    if (auto itr = active_panels.find(id); itr != active_panels.end()) {
      itr->second.panel_open = true;
      return id;
    }

    auto& p = active_panels[id] = Panel{ true, panel };
    p.panel->OnProjectChange(project_context);
    p.panel->OnAttach();

    return id;
  }

  void PanelManager::RemovePanel(const std::string& name) {
    UUID id = FNV(name);
    RemovePanel(id);
  }

  void PanelManager::RemovePanel(const UUID& panel_id) {
    auto itr = active_panels.find(panel_id);
    OE_ASSERT(itr != active_panels.end(), "Attempting to remove non-existent panel!");

    auto& panel = itr->second;
    panel.panel_open = false;
    panel.panel->OnDetach();
    panel.panel = nullptr;

    active_panels.erase(itr);
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
    std::queue<UUID> closed_panels;

    bool panel_signal = false;
    for (auto& [id, panel] : active_panels) {
      if (panel.panel == nullptr) {
        continue;
      }

      panel_signal = panel.panel->OnGuiRender(panel.panel_open) || panel_signal;

      if (id == kPropertiesPanelId && SelectionManager::HasSelection() && !panel.panel_open) {
        SelectionManager::ClearSelection();
      }

      if (!panel.panel_open) {
        closed_panels.push(id);
      }
    }

    while (!closed_panels.empty()) {
      RemovePanel(closed_panels.front());
      closed_panels.pop();
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
