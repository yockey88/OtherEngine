/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace other {

  Pipeline::Pipeline(const PipelineSpec& spec) 
      : spec(spec) {
    glGenVertexArrays(1 , &vao_id);
    glBindVertexArray(vao_id);

    vertex_buffer = NewScope<VertexBuffer>(BufferType::ARRAY_BUFFER , spec.buffer_cap);
    if (spec.has_indices) {
      index_buffer = NewScope<VertexBuffer>(BufferType::ELEMENT_ARRAY_BUFFER , spec.buffer_cap);
    }

    for (const auto& attr : spec.vertex_layout) {
      stride += attr;
    }

    uint32_t index = 0;
    uint32_t offset = 0;
    for (const auto& attr : spec.vertex_layout) {
      glEnableVertexAttribArray(index);
      glVertexAttribPointer(index , attr , GL_FLOAT , GL_FALSE , stride * sizeof(float) , (void*)(offset * sizeof(float)));

      ++index;
      offset += attr;
    }

    glBindVertexArray(0);

    target = Ref<Framebuffer>::Create(spec.framebuffer_spec);
  }

  void Pipeline::BeginRenderPass(Ref<CameraBase> camera , Ref<RenderPass> render_pass) {
  }

  void Pipeline::SubmitModel(Ref<Model> model , const glm::mat4& transform) {
  }

  void Pipeline::SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform) {
    Ref<ModelSource> model_source = model->GetModelSource();
    // const auto& submesh_data = model_source->SubMeshes();
    for (uint32_t idx : model->SubMeshes()) {
      // glm::mat4 sub_mesh_transform = transform * submesh_data[idx].transform; 
      // uint32_t mat_idx = submesh_data[idx].material;

      /// save sub_mesh_transform to apply later?
      
      /// get material

      /// build mesh key and add to current_pass data

      /// submit shadow pass draw cmd
    } 
  }

  Ref<Framebuffer> Pipeline::EndRenderPass() {
    current_pass = nullptr;
    return nullptr;
  }

} // namespace other
