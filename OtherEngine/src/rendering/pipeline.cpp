/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"

namespace std {
  
  bool less<other::MeshKey>::operator()(const other::MeshKey& lhs , const other::MeshKey& rhs) const {
    if (lhs.source_handle.Get() != rhs.source_handle.Get()) {
      return lhs.source_handle.Get() < rhs.source_handle.Get();
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

  void Pipeline::SubmitModel(Ref<Model> model , const glm::mat4& transform , const Material& material) {
  }

  void Pipeline::SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform , const Material& material) {
    Ref<ModelSource> source = model->GetModelSource();

    auto itr = std::ranges::find_if(model_submissions , [&](const auto& pair) -> bool {
      return source->handle == pair.first.source_handle;
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
        .source_handle = source->handle ,
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
    /** Passes to implement
     * ----------------
     * shadow map pass
     * spot shadow mapp pass
     * pre depth pass
     * hzb compute
     * pre integration
     * light culling
     * skybox pass
     * GTAO compute
     * GTAO denoise compute
     * AO Composite
     * pre convolution compute
     * jump flood
     * ssr compute
     * ssr composite
     * edge detection
     * bloom compute 
     * composite pass
     **/
    auto itr = model_submissions.begin();
    for (; itr != model_submissions.end();) {
      if (itr->second.instance_count == 0) {
        itr = model_submissions.erase(itr);
      } else {
        ++itr;
      }
    }

    material_storage->Clear();
    model_storage->Clear();

    gbuffer.Bind();
    RenderMeshes();
    gbuffer.Unbind();

    target->BindFrame();
    spec.lighting_shader->Bind();
    spec.target_mesh->Draw(TRIANGLES);
    spec.lighting_shader->Unbind();
    target->UnbindFrame();
   }

  Ref<Framebuffer> Pipeline::GetOutput() {
    return target;
  }
      
  GBuffer& Pipeline::GetGBuffer() {
    return gbuffer;
  }
  
  void Pipeline::Clear() {
    /// dont clear the mesh key for a tiny optimization on future submissions
    for (auto& [mk , sl] : model_submissions) {
      sl.cpu_model_storage.ZeroMem();
      sl.cpu_material_storage.ZeroMem();
      sl.instance_count = 0;
    }
    model_submissions.clear();
  }
      
  void Pipeline::RenderMeshes() {
    for (auto& [mk , sl] : model_submissions) {
      model_storage->BindBase();
      model_storage->LoadFromBuffer(sl.cpu_model_storage);
      material_storage->BindBase();
      material_storage->LoadFromBuffer(sl.cpu_material_storage);

      mk.vao->Bind();
      glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES , mk.num_elements , GL_UNSIGNED_INT , (void*)0 , sl.instance_count , 0 , 0);
    }
  }

} // namespace other
