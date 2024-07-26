/**
 * \file editor/console_panel.hpp
 **/
#ifndef OTHER_ENGINE_CONSOLE_PANEL_HPP
#define OTHER_ENGINE_CONSOLE_PANEL_HPP

#include <imgui/imgui.h>

#include "editor/editor_panel.hpp"
#include "editor/editor_console_sink.hpp"

namespace other {

  class ConsolePanel : public EditorPanel {
    public:
      ConsolePanel(Editor& editor) 
        : EditorPanel(editor) {}
      
      virtual bool OnGuiRender(bool& is_open);
      virtual void OnEvent(Event* e);
      virtual void OnProjectChange(const Ref<Project>& project);
      virtual void SetSceneContext(const Ref<Scene>& scene);

      static void PushMessage(const ConsoleMessage& message);

    private:
      uint8_t active_filters = ValOf(ALL);

      bool scroll_to_latest = true;
      float previous_scroll_h = 0.f;

      bool open_details_popup = false;

      static std::vector<ConsoleMessage> message_buffer;

      void RenderMenu(const ImVec2& size);
      void RenderConsole(const ImVec2& size);

      ImVec4 GetToolbarButtonColor(const bool value) const;
      ImVec4 GetMessageColor(const ConsoleMessage& msg) const;
      std::string GetMessageType(const ConsoleMessage& msg) const;
  };

} // namespace other

#endif // !OTHER_ENGINE_CONSOLE_PANEL_HPP
