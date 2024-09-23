/**
 * \file application/runtime_layer.cpp
 **/
#include "application/runtime_layer.hpp"

#include "core/defines.hpp"
#include "core/config_keys.hpp"
#include "core/filesystem.hpp"

#include "application/app.hpp"
#include "application/app_state.hpp"

#include "rendering/renderer.hpp"
#include "rendering/geometry_pass.hpp"

namespace other {

  void RuntimeLayer::OnAttach() {
    auto scenes = config.Get(kProjectSection , kScenesValue);
    for (const auto& scene : scenes) {
      AppState::Scenes()->LoadScene(scene);
    }

    uint32_t camera_binding_pnt = 0;
    std::vector<Uniform> cam_unis = {
      { "projection" , other::ValueType::MAT4 } ,
      { "view"       , other::ValueType::MAT4 } ,
      { "viewpoint"  , other::ValueType::VEC4 } ,
    };

    uint32_t model_binding_pnt = 1;
    std::vector<Uniform> model_unis = {
      { "models" , other::ValueType::MAT4 , 100 } ,
    };
    
    uint32_t material_binding_pnt = 2;
    std::vector<Uniform> material_unis = {
      { "materials" , other::ValueType::USER_TYPE , 100 , sizeof(other::Material) } ,
    };
    
    uint32_t light_binding_pnt = 3;
    std::vector<Uniform> light_unis = {
      { "num_lights" , other::ValueType::VEC4 } ,
      { "direction_lights" , other::ValueType::USER_TYPE , 100 , sizeof(other::DirectionLight) } ,
      { "point_lights" , other::ValueType::USER_TYPE , 100 , sizeof(other::PointLight) } ,
    };

    glm::vec2 window_size = Renderer::WindowSize();
      
    const Path shader_dir = Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders";
    const Path geom_shader_path = shader_dir / "default.oshader"; 
    std::vector<Uniform> geometry_unis = {
    };
    Ref<Shader> geometry_shader = BuildShader(geom_shader_path);
    Ref<RenderPass> geom_pass = NewRef<GeometryPass>(geometry_unis , geometry_shader);

    SceneRenderSpec spec{
      .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
      .light_uniforms = NewRef<UniformBuffer>("Lights" , light_unis , light_binding_pnt) ,
      .pipelines = {
        {
          .framebuffer_spec = {
            .depth_func = other::LESS_EQUAL ,
            .clear_color = { 0.1f , 0.1f , 0.1f , 1.f } ,
            .size =  {
              window_size.x ,
              window_size.y ,
            },
          } ,
          .vertex_layout = {
            { other::ValueType::VEC3 , "position" } ,
            { other::ValueType::VEC3 , "normal"   } ,
            { other::ValueType::VEC3 , "tangent"  } ,
            { other::ValueType::VEC3 , "binormal" } ,
            { other::ValueType::VEC2 , "uvs"      }
          } ,
          .model_uniforms = model_unis , 
          .model_binding_point = model_binding_pnt ,
          .material_uniforms = material_unis ,
          .material_binding_point = material_binding_pnt ,
          .debug_name = "Geometry" , 
        } ,
      } ,
      .passes  {
        geom_pass
      } ,
      .pipeline_to_pass_map = {
          { FNV("Geometry") , { FNV(geom_pass->Name()) } } ,
      } ,
    }; 

    scene_renderer = NewRef<SceneRenderer>(spec);
    AppState::Scenes()->StartScene();
  }

  void RuntimeLayer::OnDetach() {
    AppState::Scenes()->StopScene();
  }

  void RuntimeLayer::OnEarlyUpdate(float dt) {
    AppState::Scenes()->EarlyUpdateScene(dt);
  }

  void RuntimeLayer::OnUpdate(float dt) {
    AppState::Scenes()->UpdateScene(dt);
  }

  void RuntimeLayer::OnLateUpdate(float dt) {
    AppState::Scenes()->LateUpdateScene(dt);
  }

  void RuntimeLayer::OnRender() {
    bool scene_active = AppState::Scenes()->RenderScene(scene_renderer);
    OE_ASSERT(scene_active , "Runtime Layer rendering without scene active!");

    /// draw final frame to framebuffer
    const auto& frames = scene_renderer->GetRender();
    const auto itr = frames.find(FNV("Geometry"));
    OE_ASSERT(itr != frames.end() , "Runtime Layer rendering without pipeline!");
    const auto& [_, vp] = *itr;
    other::Renderer::DrawFramebufferToWindow(vp);
  }

  void RuntimeLayer::OnUIRender() {
    AppState::Scenes()->RenderSceneUI();
  }

} // namespace other
