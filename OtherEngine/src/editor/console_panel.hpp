/**
 * \file editor/console_panel.hpp
 **/
#ifndef OTHER_ENGINE_CONSOLE_PANEL_HPP
#define OTHER_ENGINE_CONSOLE_PANEL_HPP

#include "editor/editor_panel.hpp"

namespace other {

  class ConsolePanel : public EditorPanel {
    public:
      ConsolePanel(Editor& editor) 
        : EditorPanel(editor) {}
      
      virtual void OnGuiRender(bool& is_open) override {}
      virtual void OnEvent(Event* e) override {}
      virtual void OnProjectChange(const Ref<Project>& project) override {}
      virtual void SetSceneContext(const Ref<Scene>& scene) override {}

    private:
  };

} // namespace other

#endif // !OTHER_ENGINE_CONSOLE_PANEL_HPP
