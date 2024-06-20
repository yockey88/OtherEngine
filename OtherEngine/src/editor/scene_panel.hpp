/**
 * \file editor/scene_panel.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_PANEL_HPP
#define OTHER_ENGINE_SCENE_PANEL_HPP

#include "editor/editor_panel.hpp"

namespace other {

  class Editor;

  constexpr static std::string_view kCtxMenuFmtStr = "{}-ContextMenu";

  class ScenePanel : public EditorPanel {
    public:
      ScenePanel(Editor& parent_app) 
        : EditorPanel(parent_app) {}
      virtual ~ScenePanel() override {}

      virtual void OnGuiRender(bool& is_open) override;
      virtual void OnEvent(Event* e) override;
      virtual void OnProjectChange(const Ref<Project>& project) override;
      virtual void SetSceneContext(const Ref<Scene>& scene) override;

    private:
      Ref<Scene> active_scene;

      bool shift_selection_running = false;
      int32_t first_selected_row = -1;
      int32_t last_selected_row = -1;

      void RenderCreateEntity(Entity* parent = nullptr);
      void RenderEntity(const UUID& id , Entity* entity);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_PANEL_HPP
