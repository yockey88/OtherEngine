/**
 * \file editor/editor_panel.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_PANEL_HPP
#define OTHER_ENGINE_EDITOR_PANEL_HPP

#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "event/event.hpp"
#include "project/project.hpp"
#include "scene/scene.hpp"

namespace other {

  class Editor;

  class EditorPanel : public RefCounted {
    public:
      EditorPanel(Editor& editor) 
        : editor(editor) {}
      virtual ~EditorPanel() {}

      virtual void OnAttach() {}
      virtual void OnGuiRender(bool& is_open) = 0;
      virtual void OnEvent(Event* e) = 0;
      virtual void OnProjectChange(const Ref<Project>& project) = 0;
      virtual void SetSceneContext(const Ref<Scene>& scene) = 0;
      virtual void OnDetach() {}

    protected:
      Editor& GetEditor() const { return editor; }

    private:
      Editor& editor;
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_PANEL_HPP
