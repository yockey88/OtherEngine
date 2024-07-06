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
      
  void Pipeline::SubmitRenderPass(Ref<RenderPass> pass) {
    auto shader = pass->GetShader();
    for (auto& [id , block] : spec.uniform_blocks) {
      shader->BindToBlock(block->Name() , block->BindingPoint());
    }

    passes.push_back(pass);
  }

  void Pipeline::SubmitModel(Ref<Model> model , const glm::mat4& transform) {
  }

  void Pipeline::SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform) {
    Ref<ModelSource> source = model->GetModelSource();
    // const auto& submesh_data = model_source->SubMeshes();

    for (uint32_t sm : model->SubMeshes()) {
      MeshKey mk = {
        .model_handle = model->handle , 
        .transform = transform, // * sm[sm].transform ,
        .submesh_idx = sm ,
        .selected = false ,
      };

      static_models[mk] = Ref<StaticModel>::Clone(model);
    }
  }

  void Pipeline::Render(Ref<CameraBase>& camera) {
    auto itr = spec.uniform_blocks.find(FNV("Camera"));
    if (itr != spec.uniform_blocks.end()) {
      auto& [id , cam_block] = *itr;

      cam_block->SetUniform("projection" , camera->ProjectionMatrix());
      cam_block->SetUniform("view" , camera->ViewMatrix());
    }

    // vertex_buffer->Bind();
    // vertex_buffer->BufferData(vertices.data() , vertices.size() * sizeof(float));

    // if (spec.has_indices) {
    //   index_buffer->Bind();
    //   index_buffer->BufferData(indices.data() , indices.size() * sizeof(uint32_t));
    // }

    // CHECKGL();

    target->BindFrame();

    glBindVertexArray(vao_id);
    for (const auto& p : passes) {
      PerformPass(p);
    }
    glBindVertexArray(0);

    target->UnbindFrame();

    CHECKGL();

    // vertex_buffer->ClearBuffer();
    // if (spec.has_indices) {
    //   index_buffer->ClearBuffer();
    // }
    // 
    // vertices.clear();
    // indices.clear();

    CHECKGL();
  }

  Ref<Framebuffer> Pipeline::GetOutput() {
    return target;
  }
      
  void Pipeline::PerformPass(Ref<RenderPass> pass) {
    for (const auto& [mk , m] : static_models) {
      Ref<ModelSource> source = m->GetModelSource();

      source->BindVertexBuffer();
      if (spec.has_indices) {
        source->BindIndexBuffer();
      }

      pass->GetShader()->Bind();
      pass->SetInput("u_model" , mk.transform);

      if (spec.has_indices) {
        glDrawElements(spec.topology , indices.size() , GL_UNSIGNED_INT , 0);
      } else {
        glDrawArrays(spec.topology , 0 , vertices.size() / spec.vertex_layout.Stride()); 
      }
      
      source->UnbindVertexBuffer();
      if (spec.has_indices) {
        source->UnbindIndexBuffer();
      }
    }
  }

} // namespace other
