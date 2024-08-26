/**
 * \file rendering/pipeline.hpp
 **/
#ifndef OTHER_ENGINE_PIPELINE_HPP
#define OTHER_ENGINE_PIPELINE_HPP

#include <functional>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/buffer.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/layout.hpp"
#include "rendering/vertex.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/model.hpp"
#include "rendering/material.hpp"
#include "rendering/gbuffer.hpp"

namespace other {

  using RenderFn = std::function<void(void*)>;

  struct MeshKey {
    AssetHandle source_handle;
    Ref<VertexArray> vao = nullptr;

    size_t num_elements = 0;
    bool selected;
  };

} // namespace other

namespace std {

  template <>
  struct less<other::MeshKey> {
    bool operator()(const other::MeshKey& lhs , const other::MeshKey& rhs) const;
  };

} // namespace std

namespace other {

  struct PipelineSpec {
    DrawMode topology = DrawMode::TRIANGLES;
    bool back_face_culling = true;
    bool depth_test = true;
    bool depth_write = true;
    bool wire_frame = false;
    float line_width = 1.f;

    Ref<Shader> lighting_shader = nullptr;
    Ref<VertexArray> target_mesh = nullptr;

    FramebufferSpec framebuffer_spec {};
    Layout vertex_layout;

    std::vector<Uniform> model_uniforms{};
    uint32_t model_binding_point = 0;
    
    std::vector<Uniform> material_uniforms{};
    uint32_t material_binding_point = 0;

    std::string debug_name;
  };

  class Pipeline : public RefCounted {
    public:
      Pipeline(PipelineSpec& spec);
      virtual ~Pipeline() override {}
      
      /// FIXME: material system needs overhaul
      void SubmitModel(Ref<Model> model , const glm::mat4& transform , const Material& color);
      void SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform , const Material& color);

      void Render();
      Ref<Framebuffer> GetOutput();
      GBuffer& GetGBuffer();

      void Clear();
      
      struct MeshSubmissionList {
        uint32_t instance_count = 0;
        Buffer cpu_model_storage;
        Buffer cpu_material_storage;
      };
      
    private:
      uint32_t vao_id = 0;
      PipelineSpec spec{};
      GBuffer gbuffer;

      Ref<UniformBuffer> model_storage = nullptr;
      Ref<UniformBuffer> material_storage = nullptr;
      std::map<MeshKey , MeshSubmissionList> model_submissions;
      
      std::vector<float> vertices{};
      std::vector<uint32_t> indices{};

      uint32_t curr_transform_idx = 0;
      uint32_t idx_offset = 0;
      
      Ref<Framebuffer> target = nullptr;

      void RenderMeshes();
  };

} // namespace other

#endif // !OTHER_ENGINE_PIPELINE_HPP
