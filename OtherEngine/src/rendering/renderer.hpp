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
  };

} // namespace other

#endif // !OTHER_ENGINE_RENDERER_HPP
