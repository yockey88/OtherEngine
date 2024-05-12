/**
 * \file editor/entity_properties.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_PROPERTIES_HPP
#define OTHER_ENGINE_ENTITY_PROPERTIES_HPP

#include "editor/editor_panel.hpp"

namespace other {

  class EntityProperties : public EditorPanel {
    public:
      EntityProperties(Editor& editor)
        : EditorPanel(editor) {}
      virtual void OnGuiRender(bool& is_open) override;
      virtual void OnEvent(Event* e) override;
      virtual void OnProjectChange(const Ref<Project>& project) override;
      virtual void SetSceneContext(const Ref<Scene>& scene) override;
      
    protected:
  };

} // namespace other

#endif // !OTHER_ENGINE_ENTITY_PROPERTIES_HPP
