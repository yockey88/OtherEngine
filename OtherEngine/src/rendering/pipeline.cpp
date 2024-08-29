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
    material_storage->Clear();
    model_storage->Clear();


    /** Idea of render-pass chaining for dynamic pipeline construction
     *
     *  - implement a chaining utility to dynamically construct render pass chains 
     *      to allow for custom pipelines 
     *
     *  - let E = scene-lighting-environment
     *      accessible globally
     *
     *  - renderpass rp : X -> Y 
     *      where X is either 
     *       1 - a scene mesh/uniform set
     *       2 - a set of render buffers/textures
     *    ex:
     *      let g_rp = gbuffer_pass 
     *      then 
     *        X = { geometry-data , transforms , materials }
     *        Y_g = { position , normal , albedo , depth }
     *
     *      let l_rp = lighting_pass
     *      then 
     *        X = Y_g U { textured-quad-mesh } 
     *        Y_l = { lit-scene-texture }
     *      
     *      so that this hardcoded pipeline below cane be thought of as
     *        output-frame = l_rp o g_rp = l_rp(g_rp(X))
     *
     *  - a generic pipeline then is 
     *      pl : P -> i,
     *        P is a subset of Rp (the set of all renderpasses),
     *          P = { p_0 , ... , p_n }
     *        i \in I (the set of output images of the engine in a single run)
     *
     *      pl : p_n o ... o p_0 = p_n(...p_1(p_0(G))) = i;
     *        G = { geometry-data , transforms , materials }
     **/

    /** Pseudocode for above idea
     *
     *  - class Pipeline(pass-list) {
     *      P <- pass-list
     *    }
     *
     *    Image Pipeline::Render(scene-description) {
     *      G = { 
     *        scene-description.geometry ,
     *        scene-description.transforms ,
     *        scene-description.materials ,
     *      }
     *
     *      return execute_chain(G);
     *    }
     *
     *    Image Pipeline::execute_chain(geometry) {
     *      return fold_left(P , geometry , (G , func) { return func(G) });
     *    }
     *
     **/

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
