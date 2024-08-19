/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/rendering_defines.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/vertex.hpp"

namespace std {
  
  bool less<other::MeshKey>::operator()(const other::MeshKey& lhs , const other::MeshKey& rhs) const {
    if (lhs.model_handle.Get() != rhs.model_handle.Get()) {
      return lhs.model_handle.Get() < rhs.model_handle.Get();
    }
    
    return lhs.selected && !rhs.selected;
  }

} // namespace std

namespace other {

  Pipeline::Pipeline(PipelineSpec& s) 
      : spec(s) , gbuffer(s.framebuffer_spec.size) {
    target = Ref<Framebuffer>::Create(spec.framebuffer_spec);
    model_storage = NewRef<UniformBuffer>("ModelData" , spec.model_uniforms , spec.model_binding_point , SHADER_STORAGE);
    material_storage = NewRef<UniformBuffer>("MaterialData" , spec.material_uniforms , spec.material_binding_point , SHADER_STORAGE);
  }
      
  void Pipeline::SubmitRenderPass(Ref<RenderPass> pass) {
    pass->SetInput("goe_position" , 0);
    pass->SetInput("goe_normal" , 1);
    pass->SetInput("goe_albedo" , 2);
    passes.push_back(pass);
  }

  void Pipeline::SubmitModel(Ref<Model> model , const glm::mat4& transform , const Material& material) {
  }

  void Pipeline::SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform , const Material& material) {
    auto itr = std::ranges::find_if(model_submissions , [&](const auto& pair) -> bool {
      return model->handle == pair.first.model_handle;
    });
    

    if (itr == model_submissions.end()) {
      auto& verts = model->GetModelSource()->RawVertices();
      auto& idxs = model->GetModelSource()->Indices();

      std::vector<uint32_t> indices{};
      for (const auto& i : idxs) {
        indices.push_back(i.v1);
        indices.push_back(i.v2);
        indices.push_back(i.v3);
      }

      MeshKey key = {
        .model_handle = model->handle ,
        .vao = NewRef<VertexArray>(verts , indices) ,
      };
      key.num_elements = key.vao->NumElements();

      model_submissions[key] = {};
      itr = model_submissions.find(key);
    } 
    
    auto& [mk , sl] = *itr; 
    sl.cpu_model_storage.BufferData(transform);
    sl.cpu_material_storage.BufferData(material);
    ++sl.instance_count;
  }

  void Pipeline::Render() {
    material_storage->Clear();
    model_storage->Clear();

    gbuffer.Bind();
    RenderMeshes();
    gbuffer.Unbind();
    
    for (uint32_t i = 0; i < GBuffer::NUM_TEX_IDXS; ++i) {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D , gbuffer.textures[i]);
    }

    target->BindFrame();

    for (auto& p : passes) {
      PerformPass(p);
    }

    target->UnbindFrame();
    
    for (auto& [mk , sl] : model_submissions) {
      sl.cpu_model_storage.ZeroMem();
      sl.cpu_material_storage.ZeroMem();
      sl.instance_count = 0;
    }

    CHECKGL();
  }

  Ref<Framebuffer> Pipeline::GetOutput() {
    return target;
  }
      
  GBuffer& Pipeline::GetGBuffer() {
    return gbuffer;
  }
      
  void Pipeline::AddIndices(const Index& idx) {
    indices.push_back(idx_offset + idx.v1);
    indices.push_back(idx_offset + idx.v2);
    indices.push_back(idx_offset + idx.v3); 
  }
        
  void Pipeline::PerformPass(Ref<RenderPass>& pass) {
    pass->Bind();
    CHECKGL();

    Buffer pass_models;
    Buffer pass_materials;
    for (auto& [mk , sl] : model_submissions) {
      pass_materials = pass->ProcessMaterials(sl.cpu_material_storage);
      pass_models = pass->ProcessModels(sl.cpu_model_storage);
      
      material_storage->BindBase();
      material_storage->LoadFromBuffer(pass_materials);
      model_storage->BindBase();
      model_storage->LoadFromBuffer(pass_models);

      mk.vao->Bind();
      glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES , mk.num_elements , GL_UNSIGNED_INT , (void*)0 , sl.instance_count , 0 , 0);
    }
    CHECKGL();

    pass->Unbind();
    CHECKGL();
  }
      
  void Pipeline::RenderMeshes() {
    Buffer pass_models;
    Buffer pass_materials;
    for (auto& [mk , sl] : model_submissions) {
      material_storage->BindBase();
      material_storage->LoadFromBuffer(pass_materials);
      model_storage->BindBase();
      model_storage->LoadFromBuffer(pass_models);

      mk.vao->Bind();
      glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES , mk.num_elements , GL_UNSIGNED_INT , (void*)0 , sl.instance_count , 0 , 0);
    }
    CHECKGL();
  }

} // namespace other
