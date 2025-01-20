/**
 * \file environment/environment.cpp
 **/
#include "environment/environment.hpp"

#include <ranges>

#include "rendering/ui/ui_helpers.hpp"

namespace other {

  static ArenaAllocator<Environment> env_allocator;
  Environment* Environment::instance = nullptr;

  void Environment::Initialize() {
    Shutdown();
    instance = env_allocator.Allocate();
  }

  void Environment::Shutdown() {
    env_allocator.Free(instance);
    instance = nullptr;
  }

  void Environment::RenderTerminal() {
    auto& terminal = Get().terminal;

    ScopedColor window_bg(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
    bool open = true;
    if (ImGui::Begin("Console", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
      const ImVec2 avail = ImGui::GetContentRegionAvail();
      const ImVec2 original_cursor_pos = ImGui::GetCursorPos();

      const size_t line_h = ImGui::GetTextLineHeight();
      const float line_padding = 8.f;
      const float full_line_h = line_h + line_padding;

      /// set the cursor at the bottom of the window (minus the height of the input box)
      ImVec2 text_input_pos = { original_cursor_pos.x, avail.y };
      ImGui::SetCursorPos(text_input_pos);
      /// input box
      {
        ScopedColor text_bg_color(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
        ImVec2 size = { avail.x, static_cast<float>(line_h + 8.f) };

        if (ImGui::InputTextEx("##console:input", "[ enter command ]", terminal.input_buffer.data(), terminal.input_buffer.size(), size, 0, nullptr, nullptr)) {}
      }

      text_input_pos.x += 1.f;
      for (auto itr = terminal.terminal_history.rbegin(); itr != terminal.terminal_history.rend(); ++itr) {
        text_input_pos.y -= full_line_h;
        if (text_input_pos.y == original_cursor_pos.y) {
          break;
        }

        ImGui::SetCursorPos(text_input_pos);
        ImGui::Text("%s", itr->message.c_str());
      }
    }
    ImGui::End();
  }

  Environment& Environment::Get() {
    OE_ASSERT(instance != nullptr, "Environment not initialized");
    return *instance;
  }

  void Environment::PushTerminalMessage(const TerminalMessage& message) {
    OE_ASSERT(instance != nullptr, "Environment not initialized");
    Get().terminal.PushMessage(message);
  }

  bool Environment::ReadTerminalInput() {
    OE_ASSERT(instance != nullptr, "Environment not initialized");
    auto& terminal = Get().terminal;

    std::string input = terminal.input_buffer |
      std::views::take_while([](char c) { return c != 0; }) |
      std::ranges::to<std::string>();

    if (input.empty()) {
      std::ranges::fill(terminal.input_buffer, 0);
      return false;
    }

    PushTerminalMessage({ input, TerminalFilters::COMMAND_FILTER });

    std::ranges::fill(terminal.input_buffer, 0);
    return false;
  }

  Environment::Environment()
      : memory(), terminal(memory) {
    std::ranges::fill(terminal.input_buffer, '\0');
  }

  Environment::~Environment() {
    terminal.input_buffer = {};
  }

}  // namespace other