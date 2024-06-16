/**
 * \file rendering\renderer.hpp
 */
#ifndef OTHER_ENGINE_RENDERER_HPP
#define OTHER_ENGINE_RENDERER_HPP

#include <functional>

#include "core/ref.hpp"
#include "rendering/window.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/framebuffer.hpp"

namespace other {

  using RenderCommand = std::function<void()>;

  struct RenderData {
    Ref<CameraBase> active_camera;
    Scope<Window> window;
    std::vector<RenderCommand> commands;

    Scope<Framebuffer> viewport = nullptr;

    glm::vec4 clear_color = {0.1f, 0.1f, 0.1f, 1.0f};
  };

  class Renderer {
    public:
      static void Initialize(const ConfigTable& config);

      static void BindCamera(Ref<CameraBase>& camera);

      static void QueueCommand(RenderCommand command);
      static void BeginFrame();
      static void Render();
      static void EndFrame();
      static void SwapBuffers();

      static glm::ivec2 WindowSize();
      static void HandleWindowResize(const glm::ivec2& size);
      static bool IsWindowFocused();

      static void Shutdown();

      static const RenderData& GetData();
      static const Scope<Window>& GetWindow();

      static void ClearColor(const std::vector<std::string>& color);
      static void ClearColor(const glm::vec4& color);

    private:
      static RenderData render_data;
  };

} // namespace other

#endif // !OTHER_ENGINE_RENDERER_HPP
