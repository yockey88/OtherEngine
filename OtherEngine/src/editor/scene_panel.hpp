/**
 * \file editor/scene_panel.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_PANEL_HPP
#define OTHER_ENGINE_SCENE_PANEL_HPP

#include "editor/editor_panel.hpp"

namespace other {

  class Editor;

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

      void AcceptAssetPayload();
      void RenderSceneHierarchy(const std::map<UUID , Entity*>& entities);
      void RenderEntity(const UUID& id , Entity* entity);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_PANEL_HPP
