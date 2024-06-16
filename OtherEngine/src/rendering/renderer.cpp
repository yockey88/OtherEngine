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
    render_data.viewport = NewScope<Framebuffer>();
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
    render_data.viewport->BindFrame();
  }

  void Renderer::Render() {
    for (auto& command : render_data.commands) {
      command();
    }
    render_data.commands.clear();
  }
      
  void Renderer::EndFrame() {
    render_data.viewport->UnbindFrame();
  }
      
  void Renderer::SwapBuffers() {
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
      
  const RenderData& Renderer::GetData() { 
    return render_data; 
  }
  
  const Scope<Window>& Renderer::GetWindow() { 
    return render_data.window; 
  }

  void Renderer::ClearColor(const std::vector<std::string>& color) {
    glm::vec4 c;
    for (int i = 0; i < color.size(); i++) {
      c[i] = std::stof(color[i]);
    }
    render_data.window->SetClearColor(c);
  }
      
  void Renderer::ClearColor(const glm::vec4& color) { 
    render_data.window->SetClearColor(color); 
  }

} // namespace other
