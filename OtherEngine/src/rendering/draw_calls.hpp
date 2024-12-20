/**
 * \file rendering/draw_calls.hpp
 **/
#ifndef OTHER_ENGINE_DRAW_CALLS_HPP
#define OTHER_ENGINE_DRAW_CALLS_HPP

// #include <functional>

#include "asset/asset_defines.hpp"

#include "rendering/model.hpp"
#include "rendering/rendering_defines.hpp"

namespace other {

  // using RenderFn = std::function<void(void*)>;

  struct MeshKey {
    AssetHandle source_handle;
    RenderState render_state = RenderState::FILL;
    DrawMode draw_mode = DrawMode::TRIANGLES;

    constexpr auto operator<=>(const MeshKey&) const = default;
  };

  struct PipelineSpec {
    bool back_face_culling = true;
    bool depth_test = true;
    float line_width = 1.f;

    FramebufferSpec framebuffer_spec{};

    std::string pipeline_name;
  };

  struct RenderStaticSubmission {
    Ref<StaticModel> model = nullptr;
    glm::mat4 transform = glm::mat4(1.f);
    UUID material{};
    RenderState render_state = RenderState::FILL;
    DrawMode draw_mode = DrawMode::TRIANGLES;

    operator MeshKey() const;
  };

  struct RenderSubmission {
    Ref<Model> model = nullptr;
    glm::mat4 transform = glm::mat4(1.f);
    UUID material{};
    RenderState render_state = RenderState::FILL;
    DrawMode draw_mode = DrawMode::TRIANGLES;

    operator MeshKey() const;
  };

  struct SubMeshDrawCall {
    Buffer cpu_material_storage;

    uint32_t vertex_offset = 0;
    uint32_t vertex_count = 0;

    uint32_t index_offset = 0;
    uint32_t index_count = 0;

    uint32_t instance_count = 0;
  };

  struct MeshDrawCall {
    Ref<VertexArray> vao = nullptr;
    uint32_t base_instance = 0;
    Buffer cpu_model_storage;

    std::vector<SubMeshDrawCall> submissions;
  };

  struct StaticMeshDrawCall {
    Ref<VertexArray> vao = nullptr;
    Buffer cpu_model_storage;
    Buffer cpu_material_storage;

    uint32_t instance_count = 0;
    uint32_t index_count = 0;

    UUID material_id = 0;
  };

  using FrameMeshes = std::map<MeshKey, MeshDrawCall>;
  using StaticFrameMeshes = std::map<MeshKey, StaticMeshDrawCall>;

}  // namespace other

#endif  // !OTHER_ENGINE_DRAW_CALLS_HPP