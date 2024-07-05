/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "rendering/render_pass.hpp"

namespace other {

  Pipeline::Pipeline(const PipelineSpec& spec) 
      : spec(spec) {
    glGenVertexArrays(1 , &vao_id);
    glBindVertexArray(vao_id);

    CHECKGL();

    vertex_buffer = NewScope<VertexBuffer>(BufferType::ARRAY_BUFFER , spec.buffer_cap);
    if (spec.has_indices) {
      index_buffer = NewScope<VertexBuffer>(BufferType::ELEMENT_ARRAY_BUFFER , spec.buffer_cap);
    }

    OE_DEBUG("Setting vertex attributes");

    uint32_t index = 0;
    uint32_t offset = 0;
    uint32_t stride = spec.vertex_layout.Stride();
  
    OE_ASSERT(stride >= 0 , "Stride for pipeline [{}] is negative ({})" , spec.debug_name , stride);

    for (const auto& attr : spec.vertex_layout) {
      glEnableVertexAttribArray(index);
      glVertexAttribPointer(index , attr.size , GL_FLOAT , GL_FALSE , stride * sizeof(float) , (void*)(offset * sizeof(float)));
      CHECKGL();

      ++index;
      offset += attr.size;
    }
    
    OE_DEBUG("Set vertex attributes");

    glBindVertexArray(0);

    target = Ref<Framebuffer>::Create(spec.framebuffer_spec);
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
      vertex_buffer->Bind();
      vertex_buffer->BufferData(model_source->Vertices().data() , model_source->Vertices().size() * sizeof(float));
      curr_buffer_offset += model_source->Vertices().size();

      if (spec.has_indices) {
        index_buffer->Bind();
        index_buffer->BufferData(model_source->Indices().data() , model_source->Indices().size() * sizeof(uint32_t));
        curr_idx_buffer_offset += model_source->Indices().size();
      }

    } 
  }

  Ref<Framebuffer> Pipeline::Render(Ref<CameraBase> camera , Ref<RenderPass> render_pass) {
      glBindVertexArray(vao_id);
      if (spec.has_indices) {
        glDrawElements(spec.topology , index_buffer->Size() , GL_UNSIGNED_INT , 0);
      } else {
      }
      glBindVertexArray(0);

      vertex_buffer->ClearBuffer();
      if (spec.has_indices) {
        index_buffer->ClearBuffer();
      }


    return nullptr;
  }

} // namespace other
