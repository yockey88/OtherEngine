/**
 * \file rendering/scene_renderer.cpp
 **/
#include "rendering/scene_renderer.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/renderer.hpp"
#include <glad/glad.h>

namespace other {

  SceneRenderer::SceneRenderer(SceneRenderSpec spec) 
      : spec(spec) {
    Initialize();
  }

  SceneRenderer::~SceneRenderer() {
  
  }

  void SceneRenderer::SetViewportSize(const glm::ivec2& size) {
  }
  
  void SceneRenderer::SubmitModel(Ref<Model> model , const glm::mat4& transform) {
  }

  void SceneRenderer::SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform) {
  }

  /**
   * fn SetUniforms(camera , light)
   *    for u in camera.uniforms 
   *      passes.bind(u)
   *    for l in light
   *      passes.bind(l)
   **/
  void SceneRenderer::BeginScene(Ref<CameraBase>& camera) {
    viewpoint = camera;

    /// set uniforms
  }
  
  void SceneRenderer::EndScene() {
    FlushDrawList();
  }

  Ref<Framebuffer> SceneRenderer::GetRender() {
    return image_ir[0];
  }


  void SceneRenderer::Initialize() {
    PipelineSpec spec = {
      // .has_indices = true ,
      .framebuffer_spec = {
        .depth_func = LESS_EQUAL ,
        .clear_color = { 0.1f , 0.3f , 0.5f , 1.f } ,
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

    // pipelines.emplace_back(NewRef<Pipeline>(spec));
    
    glGenVertexArrays(1 , &vao);
    glBindVertexArray(vao);
    
    vertex_buffer = NewScope<VertexBuffer>(ARRAY_BUFFER , 4096 * sizeof(float));
    index_buffer = NewScope<VertexBuffer>(ELEMENT_ARRAY_BUFFER , 4096 * sizeof(uint32_t));
    
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

    glBindVertexArray(0);
  }

  void SceneRenderer::Shutdown() {
    pipelines.clear();
  }
      
  void SceneRenderer::FlushDrawList() {
    for (const auto& [mk , dc] : static_models) {
      /// const glm::mat4& transform = ??? 

      // auto& verts = dc->model_source->Vertices();
      // vertices.insert(vertices.end() , verts.begin() , verts.end());

      // auto& indices = dc->model_source->Indices();
      // for (auto& i : indices) {
      //   idxs.push_back(i.v1);
      //   idxs.push_back(i.v2);
      //   idxs.push_back(i.v3);
      // }
    }

    vertex_buffer->Bind();

    /// pre render
    /// begin command buffer ?
    ///
    /// shadow map pass
    /// spot shadow mapp pass
    /// pre depth pass
    /// hzb compute
    /// pre integration
    /// light culling
    /// skybox pass

    // begin pass
    // end pass

    /// GTAO compute
    /// GTAO denoise compute
    /// AO Composite
    /// pre convolution compute
    ///
    /// jump flood
    ///
    /// ssr compute
    /// ssr composite
    /// edge detection
    /// bloom compute 
    /// composite pass
    ///
    /// end command buffer
    /// submit command buffer
  }

  Ref<Framebuffer> SceneRenderer::RenderFrame(Ref<Pipeline> pipeline) {
    // set uniforms
    pipeline->Render(viewpoint);
    return pipeline->GetOutput();
  }
      
} // namespace other
