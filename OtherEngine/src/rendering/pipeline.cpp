/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

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

  Pipeline::Pipeline(PipelineSpec& spec) 
      : spec(spec) {
    target = Ref<Framebuffer>::Create(spec.framebuffer_spec);
    OE_ASSERT(spec.model_storage != nullptr , "Constructing pipeline [{}] with null model storage" , spec.debug_name);
    spec.model_storage->BindBase();
  }
      
  void Pipeline::SubmitRenderPass(const Ref<RenderPass>& pass) {
    passes.push_back(pass);
  }

  void Pipeline::SubmitModel(Ref<Model>& model , const glm::mat4& transform , const Material& material) {
  }

  void Pipeline::SubmitStaticModel(Ref<StaticModel>& model , const glm::mat4& transform , const Material& material) {
    Ref<ModelSource> source = model->GetModelSource();
    // const auto& submesh_data = model_source->SubMeshes();
    
    auto itr = std::ranges::find_if(model_submissions , [&](const auto& pair) -> bool {
      return model->handle == pair.first.model_handle;
    });

    if (itr == model_submissions.end()) {
      MeshKey key = {
        .model_handle = model->handle ,
        .vao = Ref<VertexArray>::Clone(model->GetMesh()) ,
        .num_elements = model->GetMesh()->NumElements() ,
      };

      model_submissions[key] = {};
      itr = model_submissions.find(key);
    } 
    
    /// will be valid now
    auto& [mk , sl] = *itr; 
    spec.model_storage->SetUniform("models" , transform , sl.size());

    auto& submission = sl.emplace_back();
    submission.material = material;
  }

  void Pipeline::Render() {
    target->BindFrame();

    for (auto& p : passes) {
      PerformPass(p);
    }

    target->UnbindFrame();

    model_submissions.clear();

    CHECKGL();
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

    for (auto& [mk , sl] : model_submissions) {
      mk.vao->Bind();
      glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES , mk.num_elements , GL_UNSIGNED_INT , (void*)0 , sl.size() , 0 , 0);
    }
    CHECKGL();

    pass->Unbind();
    CHECKGL();
  }

} // namespace other
