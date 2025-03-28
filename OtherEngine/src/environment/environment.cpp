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
    OE_ASSERT(instance == nullptr, "Environment already initialized");
    instance = env_allocator.Allocate();
  }

  void Environment::Shutdown() {
    env_allocator.Free(instance);
    instance = nullptr;
  }

  void Environment::RenderTerminal() {
    auto& terminal = Get().terminal;

    PROFILE_SECTION("OtherEnvironment--RenderTerminal");

    bool open = true;
    ScopedColor window_bg(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
    if (ImGui::Begin("OtherEnvironment:Terminal", &open)) {
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

        if (terminal.history_cursor.has_value()) {
          OE_ASSERT(*terminal.history_cursor < terminal.stored_history.size(), "History cursor out of bounds!");
          const TerminalMessage& message = terminal.stored_history[*terminal.history_cursor];
          std::ranges::fill(terminal.input_buffer, '\0');
          std::ranges::copy(message.message, terminal.input_buffer.begin());
        }

        // clang-format off
        if (ImGui::InputTextEx("##console:input", "[ enter command ]",
                            terminal.input_buffer.data(), terminal.input_buffer.size(),
                            size, 0, nullptr, nullptr)) {}
        // clang-format on
      }

      text_input_pos.x += 1.f;
      for (auto itr = terminal.terminal_history.rbegin(); itr != terminal.terminal_history.rend(); ++itr) {
        text_input_pos.y -= full_line_h;
        if (text_input_pos.y == original_cursor_pos.y) {
          break;
        }

        ImGui::SetCursorPos(text_input_pos);
        {
          glm::vec4 color = terminal.GetColorForFilter(itr->filters);
          ImVec4 text_color = ImVec4(color.r, color.g, color.b, color.a);
          ScopedColor text_color_scope(ImGuiCol_Text, text_color);
          ImGui::Text("%s", itr->message.c_str());
        }
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

    PushTerminalMessage({ TerminalFilter::NO_FILTER, input });

    std::ranges::fill(terminal.input_buffer, 0);
    return false;
  }

  Environment::Environment() {
    std::ranges::fill(terminal.input_buffer, '\0');
  }

  Environment::~Environment() {
    terminal.input_buffer = {};
  }

}  // namespace other