/**
 * \file rendering/scene_renderer.cpp
 **/
#include "rendering/scene_renderer.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/renderer.hpp"

namespace other {

  SceneRenderer::SceneRenderer(SceneRenderSpec spec) 
      : spec(spec) {
    Initialize();
  }

  SceneRenderer::~SceneRenderer() {
  
  }

  void SceneRenderer::SetViewportSize(const glm::ivec2& size) {
    pipelines.clear();

    PipelineSpec spec = {
      .framebuffer_spec = {
        .depth_func = LESS_EQUAL ,
        .clear_color = { 0.f , 1.f , 0.f , 1.f } ,
        .size = size ,
      } ,
      .vertex_layout = {
    	{ ValueType::VEC3, "position" },
    	{ ValueType::VEC3, "normal" },
    	{ ValueType::VEC3, "tangent" },
    	{ ValueType::VEC3, "binormal" },
    	{ ValueType::VEC2, "uvs" }
      } ,
      .debug_name = "Testing Pipeline" , 
    };

    pipelines.emplace_back(NewRef<Pipeline>(spec));
  }
  
  void SceneRenderer::SubmitModel(Ref<Model> model , const glm::mat4& transform) {
    Ref<ModelSource> source = model->GetModelSource();
    const auto& submeshes = source->SubMeshes();

    for (uint32_t sm : model->SubMeshes()) {
      // tform = transform * sm[sm].transform
      //
      // /* do material stuff */

      MeshKey mk = {
        model->handle , 
        sm ,
        false ,
      };

      models[mk] = model; 
    }
  }

  void SceneRenderer::SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform) {
    Ref<ModelSource> source = model->GetModelSource();
    const auto& submeshes = source->SubMeshes();

    OE_DEBUG("Submitting mesh with {} submeshes ({} claimed)" , submeshes.size() , model->SubMeshes().size());
    for (uint32_t sm : model->SubMeshes()) {
      // tform = transform * sm[sm].transform
      //
      // /* do material stuff */

      MeshKey mk = {
        model->handle , 
        sm ,
        false ,
      };

      OE_DEBUG("Submitting [{}] for render" , model->handle);
      static_models[mk] = model; 
    }
  }

  /**
   * fn SetUniforms(camera , light)
   *    for u in camera.uniforms 
   *      passes.bind(u)
   *    for l in light
   *      passes.bind(l)
   **/

  Ref<Framebuffer> SceneRenderer::RenderScene(Ref<CameraBase>& camera) {
    viewpoint = camera;

    /// for p in pipelines
    ///   Clear()
    ///
    /// SetUniforms(camera , lighting)
    ///
    /// for p in pipelines begin(pipelines , pipeline_builder) {
    ///   for m in models and static models {
    ///     p.submit(m)
    ///   }

    /// need to be able to submit mesh to correct pipeline
    ///   based on input

    for (auto& p : pipelines) {
      for (auto& [key , m] : models) {
        p->SubmitModel(m , glm::mat4(1.f));
      }  
      for (auto& [key , m] : static_models) {
        p->SubmitModel(m , glm::mat4(1.f));
      }  
    }

    // wait for images
    // geometry_pass->Input(...);
    for (auto& p : pipelines) {
      image_ir.push_back(RenderFrame(p)); 
    }

    if (image_ir.empty()) { 
      OE_DEBUG("No image to draw");
      return nullptr;
    }

    if (image_ir[0] == nullptr) {
      return nullptr;
    }


    return image_ir[0]; 

    /// frame = PostProcess(image_ir)
    ///
    /// return frame
  }

  void SceneRenderer::Initialize() {
    PipelineSpec spec = {
      .framebuffer_spec = {
        .depth_func = LESS_EQUAL ,
        .clear_color = { 0.f , 1.f , 0.f , 1.f } ,
        .size = Renderer::WindowSize() ,
      } ,
      .vertex_layout = {
    	{ ValueType::VEC3, "position" },
    	{ ValueType::VEC3, "normal" },
    	{ ValueType::VEC3, "tangent" },
    	{ ValueType::VEC3, "binormal" },
    	{ ValueType::VEC2, "uvs" }
      } ,
      .debug_name = "Testing Pipeline" , 
    };

    pipelines.emplace_back(NewRef<Pipeline>(spec));
  }

  void SceneRenderer::Shutdown() {
    pipelines.clear();
  }
      

  Ref<Framebuffer> SceneRenderer::RenderFrame(Ref<Pipeline> pipeline) {
    /**
     *  output : framebuffer = undef
     *  output = framebuffer.pass(geometry_pass);
     *  ... more passes
     *  output = framebuffer.pass(xpass);
     *
     *  return output;
     **/
    return pipeline->Render(viewpoint , geometry_pass);
  }
      
} // namespace other
