/**
 * \file rendering\renderer.hpp
 */
#ifndef OTHER_ENGINE_RENDERER_HPP
#define OTHER_ENGINE_RENDERER_HPP

#include <array>
#include <span>
#include <queue>

#include "core/ref.hpp"
#include "scene/scene.hpp"

#include "rendering/window.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/framebuffer.hpp"

#include "rendering/model.hpp"

namespace other {

  constexpr static uint32_t kNumPrimitives = 3;
  constexpr static uint32_t kLineBatchIndex = 0;
  constexpr static uint32_t kTriangleBatchIndex = 1;
  constexpr static uint32_t kRectBatchIndex = 2;
  
  struct RenderData {
    /// global data
    Scope<Window> window;

    /// frame data
    bool frame_bound = false;
    Ref<Framebuffer> frame = nullptr;
    glm::vec4 clear_color = {0.1f, 0.1f, 0.1f, 1.0f};

    /// scene context data
    Ref<Scene> scene_ctx;
    Ref<CameraBase> active_camera;

    std::queue<StaticModel> model_queue;
  };

  struct UniformBuffer {
    UUID hash;
    std::string name;
    /// DataType type;
  };

  struct RenderPass {
  };
  
  class Pipeline {
    Ref<CameraBase> frame_viewpoint;
    Ref<Framebuffer> target = nullptr;
    std::map<UUID , RenderPass> passes;
  };

  class Renderer {
    public:
      static void Initialize(const ConfigTable& config);

      static void SetSceneContext(const Ref<Scene>& scene);
      static void BindCamera(Ref<CameraBase>& camera);

      static void BeginFrame();

      /*
      static void DrawLine(const Point& start , const Point& end , const RgbColor& color);
      static void DrawLine(const Vertex& start , const Vertex& end);
      static void DrawLine(const Line& line , const RgbColor& color);

      static void DrawTriangle(const Point& p1 , const Point& p2 , const Point& p3 , const RgbColor& color);
      static void DrawTriangle(const std::array<Vertex , 3>& corners);
      static void DrawTriangle(const Triangle& triangle , const RgbColor& color);
      
      static void DrawRect(const Point& position , const glm::vec2& half_extents , const RgbColor& color);
      static void DrawRect(const std::span<Vertex , 4>& corners);
      static void DrawRect(const Rect& rect , const RgbColor& color);

      static void DrawStaticModel(Ref<StaticModel> cube_model);

      static void DrawCube(const Point& position , const glm::vec3& half_extents);
      static void DrawCube(const std::span<Vertex , 8>& corners);
      static void DrawCube(const Cube& cube , const RgbColor& color);

      static void DrawCircle(const Point& position , float radius);
      static void DrawCircle(const Circle& circle , const RgbColor& color);

      static void DrawSphere(const Point& position , float radius);
      static void DrawSphere(const Sphere& circle , const RgbColor& color);

      static void DrawCapsule(const Capsule& capsule , const RgbColor& color);

      static void DrawCylinder(const Cylinder& cylinder , const RgbColor& color);
      */

      static void EndFrame();
      static void SwapBuffers();

      static glm::ivec2 WindowSize();
      static void HandleWindowResize(const glm::ivec2& size);
      static bool IsWindowFocused();

      static void Shutdown();

      static const Scope<Window>& GetWindow();
      static Ref<Framebuffer>& Viewport();

      static void ClearColor(const std::vector<std::string>& color);
      static void ClearColor(const glm::vec4& color);

    private:
      static RenderData render_data;

      // static void WriteVertexToBatch(Batch& batch , const Vertex& vert);
      static void WriteRectIndices();

      // static void RenderBatch(Batch& batch);
      
      // static void CreateRenderBatch(const BatchData& data , uint32_t idx , bool internal = false);
      // static Batch CreateRenderBatch(const BatchData& data);

      static void CreatePrimitiveBatches();
      static void DestroyPrimitiveBatches();
      static void DestroyBatches();
  };

} // namespace other

#endif // !OTHER_ENGINE_RENDERER_HPP
