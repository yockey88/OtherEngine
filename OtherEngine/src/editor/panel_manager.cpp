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

  constexpr static uint32_t kNumDefaultPanels = 5;

  using PanelBuilder = Ref<EditorPanel> (*)();
  using PanelBuilderPair = std::pair<UUID, PanelBuilder>;

  constexpr static std::array<PanelBuilderPair, kNumDefaultPanels> kPanelBuilderMap{
    PanelBuilderPair{
      Panel::kProjectPanelId,
      []() -> Ref<EditorPanel> { return NewRef<ProjectPanel>(); },
    },
    PanelBuilderPair{
      Panel::kScenePanelId,
      []() -> Ref<EditorPanel> { return NewRef<ScenePanel>(); },
    },
    PanelBuilderPair{
      Panel::kPropertiesPanelId,
      []() -> Ref<EditorPanel> { return NewRef<EntityProperties>(); },
    },
    PanelBuilderPair{
      Panel::kConsolePanelId,
      []() -> Ref<EditorPanel> { return NewRef<LogPanel>(); },
    },
    PanelBuilderPair{
      Panel::kViewportPanelId,
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

  void PanelManager::OpenPanel(const UUID& panel_id) {
    if (auto itr = active_panels.find(panel_id); itr != active_panels.end()) {
      itr->second.panel_open = true;
    }
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
  }

  void PanelManager::Update(float dt) {
    for (auto& [id, panel] : active_panels) {
      if (panel.panel == nullptr) {
        continue;
      }

      panel.panel->OnUpdate(dt);
    }
  }

  void PanelManager::LateUpdate(float dt) {
    for (auto& [id, panel] : active_panels) {
      if (panel.panel == nullptr) {
        continue;
      }

      panel.panel->OnLateUpdate(dt);
    }
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

      /// dont remove these panels
      if (id == Panel::kProjectPanelId || id == Panel::kScenePanelId || id == Panel::kPropertiesPanelId ||
          id == Panel::kConsolePanelId || id == Panel::kViewportPanelId) {
        continue;
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
