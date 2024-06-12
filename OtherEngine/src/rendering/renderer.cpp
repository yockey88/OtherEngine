/**
 * \file rendering\renderer.cpp
 */
#include "rendering\renderer.hpp"

#include "core\logger.hpp"

#include "event/core_events.hpp"
#include "event\event_queue.hpp"

namespace other {

  RenderData Renderer::render_data;

  void Renderer::Initialize(const ConfigTable& config) {
    auto win_cfg = Window::ConfigureWindow(config);
    auto win_res = Window::GetWindow(win_cfg, config);
    if (win_res.IsErr()) {
      OE_CRITICAL("Failed to create window : {}", win_res.Error());
      EventQueue::PushEvent<ShutdownEvent>(ExitCode::FAILURE);
      return;
    }

    render_data.window = std::move(win_res.Unwrap());
  }

  void Renderer::QueueCommand(RenderCommand command) {
    if (command == nullptr) {
      OE_WARN("Attempting to queue a null command");
      return;
    }

    render_data.commands.push_back(command);
  }

  void Renderer::BeginFrame() {
    render_data.window->Clear();
  }

  void Renderer::Render() {
    for (auto& command : render_data.commands) {
      command();
    }
    render_data.commands.clear();
  }

  void Renderer::EndFrame() {
    render_data.window->SwapBuffers();
  }

  glm::ivec2 Renderer::WindowSize() {
    return render_data.window->Size();
  }

  void Renderer::HandleWindowResize(const glm::ivec2& size) {
    render_data.window->Resize(size);
  }
      
  bool Renderer::IsWindowFocused() {
    return render_data.window->HasFocus();
  }

  void Renderer::Shutdown() {
    render_data.window = nullptr;
  }

} // namespace other
