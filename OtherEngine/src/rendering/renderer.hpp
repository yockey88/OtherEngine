/**
 * \file rendering\renderer.hpp
 */
#ifndef OTHER_ENGINE_RENDERER_HPP
#define OTHER_ENGINE_RENDERER_HPP

#include "core/ref.hpp"
#include "scene/scene.hpp"

#include "rendering/window.hpp"
#include "rendering/framebuffer.hpp"

namespace other {

  class Renderer {
    public:
      static void Initialize(const ConfigTable& config);

      static void SetSceneContext(const Ref<Scene>& scene);

      static glm::ivec2 WindowSize();
      static glm::ivec2 WindowPos();

      static void HandleWindowResize(const glm::ivec2& size);
      static bool IsWindowFocused();

      static void Shutdown();

      static const Scope<Window>& GetWindow();

      static void SetWindowClearColor(const std::vector<std::string>& color);
      static void SetWindowClearColor(const glm::vec4& color);

      static void DrawFramebufferToWindow(const Ref<Framebuffer>& framebuffer);

    private:
      static Scope<Window> window;
      static Ref<Scene> scene_ctx;
      static Ref<VertexArray> window_mesh;
      static Ref<Shader> window_shader;
  };

} // namespace other

#endif // !OTHER_ENGINE_RENDERER_HPP
