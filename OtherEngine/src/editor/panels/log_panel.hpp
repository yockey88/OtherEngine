/**
 * \file editor/panels/log_panel.hpp
 **/
#ifndef OTHER_ENGINE_LOG_PANEL_HPP
#define OTHER_ENGINE_LOG_PANEL_HPP

#include <imgui/imgui.h>

#include "editor/editor_panel.hpp"
#include "editor/editor_sink.hpp"
#include "environment/terminal.hpp"

#include "event/key_events.hpp"

namespace other {

  class LogPanel : public EditorPanel {
   public:
    LogPanel() {}
    virtual ~LogPanel() override;

    void OnAttach() override;
    bool OnGuiRender(bool& is_open) override;

    void OnLateUpdate(float dt) override;

    void OnProjectChange(const Ref<Project>& project) override;
    void SetSceneContext(const Ref<Scene>& scene) override;

    static void PushMessage(const ConsoleMessage& message);

   private:
    uint32_t active_filters = ValOf(ALL);

    bool scroll_to_latest = true;
    float previous_scroll_h = 0.f;

    bool open_details_popup = false;

    // constexpr static size_t kBufferSize = 4096;
    // std::array<char, kBufferSize> input_buffer;

    // std::vector<TerminalMessage> terminal_buffer;
    static std::vector<ConsoleMessage> message_buffer;

    void RenderMenu(const ImVec2& size);
    void RenderLog(const ImVec2& size);

    ImVec4 GetToolbarButtonColor(const bool value) const;
    ImVec4 GetMessageColor(const ConsoleMessage& msg) const;
    std::string GetMessageType(const ConsoleMessage& msg) const;

    bool HandleKeyEvent(KeyPressed& event);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_LOG_PANEL_HPP
