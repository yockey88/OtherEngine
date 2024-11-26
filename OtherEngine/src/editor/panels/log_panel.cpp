/**
 * \file editor/panels/log_panel.cpp
 **/
#include "editor/panels/log_panel.hpp"

#include <algorithm>
#include <ranges>

#include <imgui/imgui.h>

#include "editor/editor_sink.hpp"
#include "environment/environment.hpp"

#include "event/event_queue.hpp"
#include "input/keyboard.hpp"

#include "rendering/renderer.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {

  std::vector<ConsoleMessage> LogPanel::message_buffer;

  /// TODO: change debug tint
  constexpr static ImVec4 kDebugTint = ImVec4(0.f, 0.431372549f, 1.f, 1.f);
  constexpr static ImVec4 kInfoTint = ImVec4(0.047058823f, 0.56470588f, 0.039215686f, 1.f);
  constexpr static ImVec4 kWarningTint = ImVec4(1.f, 0.890196078f, 0.0588235294f, 1.f);
  constexpr static ImVec4 kErrorTint = ImVec4(1.f, 0.309803922f, 0.309903922f, 1.f);

  LogPanel::~LogPanel() {
    EventQueue::UnregisterEventDispatcher("ConsolePanel--KeyPressed");

    message_buffer.clear();
  }

  void LogPanel::OnAttach() {
    EventQueue::RegisterEventDispatcher<KeyPressed>(
      "ConsolePanel--KeyPressed",
      { std::bind_front(&LogPanel::HandleKeyEvent, this) }
    );
  }

  bool LogPanel::OnGuiRender(bool& is_open) {
    if (ImGui::Begin("Log", &is_open)) {
      ImVec2 size = ImGui::GetContentRegionAvail();
      size.y -= 32.f;

      RenderMenu({ size.x, 28.f });
      RenderLog(size);
    }
    ImGui::End();

    /// never should have to signal changes to panel manager
    return false;
  }

  void LogPanel::OnLateUpdate(float dt) {
  }

  void LogPanel::OnProjectChange(const Ref<Project>& project) {
    message_buffer.clear();
  }

  void LogPanel::SetSceneContext(const Ref<Scene>& scene) {
    message_buffer.clear();
  }

  void LogPanel::PushMessage(const ConsoleMessage& message) {
    message_buffer.push_back(message);
  }

  void LogPanel::RenderMenu(const ImVec2& size) {
    ScopedStyle frame(ImGuiStyleVar_FrameBorderSize, 0.f);
    ScopedStyle window_padding(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

    ImGui::BeginChild("Toolbar", size);

    const float tool_bar_h = size.y;

    if (ImGui::Button("Clear", { 75.f, size.y })) {
      message_buffer.clear();
    }

    ImGui::SameLine();

    const auto& style = ImGui::GetStyle();

    ImVec4 text_col = style.Colors[ImGuiCol_Text];

    /// clear on play toggle
    // const std::string clear = fmt::format("{} Clear On Play" , clear;

    /// progress label?

    {
      const ImVec2 button_sz{ tool_bar_h, tool_bar_h };

      bool debug_filter = active_filters & ValOf(DEBUG_FILTER);
      bool info_filter = active_filters & ValOf(INFO_FILTER);
      bool warning_filter = active_filters & ValOf(WARNING_FILTER);
      bool error_filter = active_filters & ValOf(ERR_FILTER);

      ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100.f, 0.f);
      text_col = debug_filter ?
        kDebugTint :
        style.Colors[ImGuiCol_TextDisabled];
      /// circle icon in fontawesome-webfont.ttf = u8"\uf05a"
      if (ui::ColoredButton("o", GetToolbarButtonColor(debug_filter), text_col, button_sz)) {
        active_filters ^= ValOf(DEBUG_FILTER);
      }

      ImGui::SameLine();
      text_col = info_filter ?
        kInfoTint :
        style.Colors[ImGuiCol_TextDisabled];
      /// exlamation icon in fontawesome-webfont.ttf = ???
      if (ui::ColoredButton("i", GetToolbarButtonColor(info_filter), text_col, button_sz)) {
        active_filters ^= ValOf(INFO_FILTER);
      }

      ImGui::SameLine();
      text_col = warning_filter ?
        kWarningTint :
        style.Colors[ImGuiCol_TextDisabled];
      /// triangle exclamation icon in fontawesome-webfont.ttf = u8"\uf071"
      if (ui::ColoredButton("!", GetToolbarButtonColor(warning_filter), text_col, button_sz)) {
        active_filters ^= ValOf(WARNING_FILTER);
      }

      ImGui::SameLine();
      text_col = error_filter ?
        kErrorTint :
        style.Colors[ImGuiCol_TextDisabled];
      /// circle exclamation icon in fontawesome-webfont.ttf = u8"\uf06a"
      if (ui::ColoredButton("x", GetToolbarButtonColor(error_filter), text_col, button_sz)) {
        active_filters ^= ValOf(ERR_FILTER);
      }
    }

    ImGui::EndChild();
  }

  void LogPanel::RenderLog(const ImVec2& size) {
    static const std::vector<std::string> columns = {
      "Type",
      "Timestamp",
      "Message",
    };

    ui::Table("Log##Table", columns, size, 3, [this]() {
      float scroll_y = ImGui::GetScrollY();
      if (scroll_y < previous_scroll_h) {
        scroll_to_latest = false;
      }

      if (scroll_y >= ImGui::GetScrollMaxY()) {
        scroll_to_latest = true;
      }

      previous_scroll_h = scroll_y;

      float row_h = 24.f;
      for (uint32_t i = 0; i < message_buffer.size(); ++i) {
        const auto& msg = message_buffer[i];

        if (!(msg.filters & active_filters)) {
          continue;
        }

        ImGui::PushID(&msg);

        /// why this no work???
        const bool clicked = ui::TableRowClickable(msg.display_msg.c_str(), row_h);

        ui::Separator(ImVec2(4.f, ImGui::CalcTextSize(msg.display_msg.c_str()).y), GetMessageColor(msg));
        ImGui::SameLine();
        ImGui::Text("%s", GetMessageType(msg).c_str());
        ImGui::TableNextColumn();
        ui::ShiftCursorX(4.f);

        std::stringstream ss;
        tm* time_buffer = localtime(&msg.time);
        ss << std::put_time(time_buffer, "%T");
        ImGui::Text("%s", ss.str().c_str());

        ImGui::TableNextColumn();
        ui::ShiftCursorX(4.f);
        ImGui::Text("%s", msg.display_msg.c_str());

        if (i == message_buffer.size() - 1 && scroll_to_latest) {
          ImGui::ScrollToItem();
          scroll_to_latest = false;
        }

        if (clicked) {
          ImGui::OpenPopup("Message Details");
          auto win_size = Renderer::WindowSize();
          auto win_pos = Renderer::WindowPos();

          ImGui::SetNextWindowSize({ (float)win_size.x * 0.5f, (float)win_size.y * 0.5f });
          ImGui::SetNextWindowPos({ (float)win_pos.x + (float)win_size.x / 2.f, (float)win_pos.y + (float)win_size.y / 2.5f }, 0, { 0.5f, 0.5f });

          open_details_popup = true;
        }

        if (open_details_popup) {
          ScopedStyle window_padding(ImGuiStyleVar_WindowPadding, ImVec2(4.f, 4.f));
          ScopedStyle frame_passing(ImGuiStyleVar_FramePadding, ImVec2(4.f, 8.f));

          if (ImGui::BeginPopupModal("Message Details", &open_details_popup, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            ImGui::TextWrapped("%s", msg.message.c_str());
            if (ImGui::Button("Copy", ImVec2(120.f, 28.f))) {
              ImGui::SetClipboardText(msg.message.c_str());
            }

            ImGui::EndPopup();
          }
        }

        ImGui::PopID();
      }
    });
  }

  ImVec4 LogPanel::GetToolbarButtonColor(const bool value) const {
    const auto& style = ImGui::GetStyle();
    return value ?
      style.Colors[ImGuiCol_Header] :
      style.Colors[ImGuiCol_FrameBg];
  }

  ImVec4 LogPanel::GetMessageColor(const ConsoleMessage& msg) const {
    if (msg.filters & ValOf(INFO_FILTER)) {
      return kInfoTint;
    }
    if (msg.filters & ValOf(WARNING_FILTER)) {
      return kWarningTint;
    }
    if (msg.filters & ValOf(ERR_FILTER)) {
      return kErrorTint;
    }
    return kDebugTint;
  }

  std::string LogPanel::GetMessageType(const ConsoleMessage& msg) const {
    if (msg.filters & ValOf(DEBUG_FILTER)) {
      return "Debug";
    }
    if (msg.filters & ValOf(INFO_FILTER)) {
      return "Info";
    }
    if (msg.filters & ValOf(WARNING_FILTER)) {
      return "Warning";
    }
    if (msg.filters & ValOf(ERR_FILTER)) {
      return "Error";
    }
    return "Unknown Message Type";
  }

  bool LogPanel::HandleKeyEvent(KeyPressed& event) {
    // if (event.Key() == Keyboard::Key::OE_RETURN) {
    //   std::string input = input_buffer |
    //     std::views::take_while([](char c) { return c != 0; }) |
    //     std::ranges::to<std::string>();

    //   if (input.empty()) {
    //     std::ranges::fill(input_buffer, 0);
    //     return false;
    //   }

    //   TerminalMessage message = { input, TerminalFilters::COMMAND_FILTER };
    //   terminal_buffer.push_back(message);
    //   Environment::PushTerminalMessage(message);

    //   std::ranges::fill(input_buffer, 0);
    //   return false;
    // }

    return false;
  }

}  // namespace other
