/**
 * \file rendering\renderer.hpp
 */
#ifndef OTHER_ENGINE_RENDERER_HPP
#define OTHER_ENGINE_RENDERER_HPP

#include <functional>

#include "rendering/window.hpp"

namespace other {

  using RenderCommand = std::function<void()>;

  struct RenderData {
    Scope<Window> window;
    std::vector<RenderCommand> commands;

    glm::vec4 clear_color = {0.1f, 0.1f, 0.1f, 1.0f};
  };

  class Renderer {
    public:
      static void Initialize(const ConfigTable& config);

      static void QueueCommand(RenderCommand command);
      static void BeginFrame();
      static void Render();
      static void EndFrame();

      static glm::ivec2 WindowSize();
      static void HandleWindowResize(const glm::ivec2& size);

      static void Shutdown();

      static const RenderData& GetData() { return render_data; }
      static const Scope<Window>& GetWindow() { return render_data.window; }

      static void ClearColor(const std::vector<std::string>& color) {
        glm::vec4 c;
        for (int i = 0; i < color.size(); i++) {
          c[i] = std::stof(color[i]);
        }
        render_data.window->SetClearColor(c);
      }
      static void ClearColor(const glm::vec4& color) { render_data.window->SetClearColor(color); }

    private:
      static RenderData render_data;
  };

} // namespace other

#endif // !OTHER_ENGINE_RENDERER_HPP
