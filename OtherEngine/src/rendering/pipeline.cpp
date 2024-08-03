/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "rendering/render_pass.hpp"

namespace other {

  Pipeline::Pipeline(PipelineSpec& spec) 
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
    spec.model_storage->BindBase();
  }
      
  void Pipeline::SubmitRenderPass(const Ref<RenderPass>& pass) {
    passes.push_back(pass);
  }

  void Pipeline::SubmitModel(Ref<Model>& model , const glm::mat4& transform) {
  }

  void Pipeline::SubmitStaticModel(Ref<StaticModel>& model , const glm::mat4& transform) {
    Ref<ModelSource> source = model->GetModelSource();
    // const auto& submesh_data = model_source->SubMeshes();
     
    auto& verts = source->RawVertices();
    auto& idxs = source->Indices();

    // uint32_t floats_added = 0;
    // for (uint32_t i = 0; i < verts.size();) {
    //   for (uint32_t j = 0; j < spec.vertex_layout.Stride(); ++j, ++floats_added) {
    //     vertices.push_back(verts[i]);
    //     ++i;
    //   }

    //   vertices.push_back(curr_transform_idx);
    // }

    // for (auto& i : idxs) {
    //   AddIndices(i);
    // }
    // 
    // spec.model_storage->SetUniform("models" , transform , curr_transform_idx);

    // ++curr_transform_idx;
    // idx_offset += floats_added; 

    std::vector<uint32_t> indices{};
    for (const auto& i : idxs) {
      indices.push_back(i.v1);
      indices.push_back(i.v2);
      indices.push_back(i.v3);
    }

    meshes.emplace_back(NewRef<VertexArray>(verts , indices));
    transforms.push_back(transform);
  }

  void Pipeline::Render() {
    // vertex_buffer->Bind();
    // vertex_buffer->BufferData(vertices.data() , vertices.size() * sizeof(float));

    // CHECKGL();

    // if (spec.has_indices) {
    //   index_buffer->Bind();
    //   index_buffer->BufferData(indices.data() , indices.size() * sizeof(uint32_t));
    // }

    // CHECKGL();

    target->BindFrame();

    // glBindVertexArray(vao_id);
    for (auto& p : passes) {
      PerformPass(p);
    }
    // glBindVertexArray(0);

    target->UnbindFrame();

    CHECKGL();

    // vertex_buffer->ClearBuffer();
    // if (spec.has_indices) {
    //   index_buffer->ClearBuffer();
    // }
    // 
    // vertices.clear();
    // indices.clear();
    meshes.clear();
    transforms.clear();

    // curr_transform_idx = 0;
    // idx_offset = 0;

    // CHECKGL();
  }

  Ref<Framebuffer> Pipeline::GetOutput() {
    return target;
  }
      
  void Pipeline::AddIndices(const Index& idx) {
    indices.push_back(idx_offset + idx.v1);
    indices.push_back(idx_offset + idx.v2);
    indices.push_back(idx_offset + idx.v3); 
  }
        
  void Pipeline::PerformPass(Ref<RenderPass>& pass) {
    pass->Bind();
    CHECKGL();

    for (uint32_t i = 0; i < meshes.size(); ++i) {
      pass->SetInput("voe_model" , transforms[i]);
      meshes[i]->Draw(spec.topology);
    }

    pass->Unbind();
    CHECKGL();

    // if (spec.has_indices) {
    //   glDrawElements(spec.topology , indices.size() , GL_UNSIGNED_INT , 0);
    // } else {
    //   glDrawArrays(spec.topology , 0 , vertices.size() / spec.vertex_layout.Stride()); 
    // }

    CHECKGL();
  }

} // namespace other
