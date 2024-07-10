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

    /// we always add an index into the end of the layout
    uint32_t stride = spec.vertex_layout.Stride() * sizeof(float) + sizeof(uint32_t);
  
    OE_ASSERT(stride >= 0 , "Stride for pipeline [{}] is negative ({})" , spec.debug_name , stride);

    for (const auto& attr : spec.vertex_layout) {
      glEnableVertexAttribArray(index);
      glVertexAttribPointer(index , attr.size , GL_FLOAT , GL_FALSE , stride , (void*)(offset * sizeof(float)));
      CHECKGL();

      ++index;
      offset += attr.size;
    }

    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index , 1 , GL_INT , GL_FALSE , stride , (void*)(offset * sizeof(float)));
    
    OE_DEBUG("Set vertex attributes");

    glBindVertexArray(0);

    target = Ref<Framebuffer>::Create(spec.framebuffer_spec);
  }
      
  void Pipeline::SubmitRenderPass(const Ref<RenderPass>& pass) {
    passes.push_back(pass);
  }

  void Pipeline::SubmitModel(Ref<Model>& model , uint32_t transform_idx) {
  }

  void Pipeline::SubmitStaticModel(Ref<StaticModel>& model , uint32_t transform_idx) {
    Ref<ModelSource> source = model->GetModelSource();
    // const auto& submesh_data = model_source->SubMeshes();
    
    auto& verts = source->Vertices();
    auto& idxs = source->Indices();

    for (uint32_t i = 0; i < verts.size();) {
      for (uint32_t j = 0; j < spec.vertex_layout.Stride(); ++j) {
        vertices.push_back(verts[i]);
        ++i;
      }

      vertices.push_back(transform_idx);
    }

    for (auto& i : idxs) {
      indices.push_back(i.v1);
      indices.push_back(i.v2);
      indices.push_back(i.v3);
    }
  }

  void Pipeline::Render() {
    vertex_buffer->Bind();
    vertex_buffer->BufferData(vertices.data() , vertices.size() * sizeof(float));

    CHECKGL();

    if (spec.has_indices) {
      index_buffer->Bind();
      index_buffer->BufferData(indices.data() , indices.size() * sizeof(uint32_t));
    }

    CHECKGL();

    target->BindFrame();

    glBindVertexArray(vao_id);
    for (auto& p : passes) {
      PerformPass(p);
    }
    glBindVertexArray(0);

    target->UnbindFrame();

    CHECKGL();

    vertex_buffer->ClearBuffer();
    if (spec.has_indices) {
      index_buffer->ClearBuffer();
    }
    
    vertices.clear();
    indices.clear();

    CHECKGL();
  }

  Ref<Framebuffer> Pipeline::GetOutput() {
    return target;
  }
        
  void Pipeline::PerformPass(Ref<RenderPass>& pass) {
    pass->Bind();
    CHECKGL();

    if (spec.has_indices) {
      glDrawElements(spec.topology , indices.size() , GL_UNSIGNED_INT , 0);
    } else {
      glDrawArrays(spec.topology , 0 , vertices.size() / spec.vertex_layout.Stride()); 
    }

    CHECKGL();
  }

} // namespace other
