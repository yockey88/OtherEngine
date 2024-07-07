/**
 * \file sandbox/main.cpp
 **/
#include "core/defines.hpp"

#include <SDL.h>
#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/engine.hpp"
#include "input/io.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"
#include "parsing/shader_compiler.hpp"

#include "application/app.hpp"
#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"

#include "event/event_queue.hpp"

#include "scripting/script_engine.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/window.hpp"
#include "rendering/renderer.hpp"
#include "rendering/shader.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/uniform.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/scene_renderer.hpp"
#include "rendering/ui/ui.hpp"

static constexpr std::string_view vert1 = R"(
#version 460 core

layout (location = 0) in vec3 vpos;
layout (location = 1) in vec3 vnorm;
layout (location = 2) in vec3 vtan;
layout (location = 3) in vec3 vbitan;
layout (location = 4) in vec2 vuvs;
layout (location = 5) in int model_id;
 
layout (std140) uniform Camera {
  mat4 projection;
  mat4 view;
};
 
layout (std430 , binding = 1) readonly buffer ModelData {
  mat4 models[];
};
 
out vec4 fcol;

void main() {
  gl_Position = projection * view * models[model_id] * vec4(vpos , 1.0);
  fcol = vec4(vpos , 1.0);
}
)";

static constexpr std::string_view frag1 = R"(
#version 460 core

in vec4 fcol;
out vec4 frag_color;

void main() {
  frag_color = fcol;
}
)";

static constexpr std::string_view vert2 = R"(
#version 460 core

layout (location = 0) in vec3 vpos;
layout (location = 1) in vec3 vnorm;
layout (location = 2) in vec3 vtan;
layout (location = 3) in vec3 vbitan;
layout (location = 4) in vec2 vuvs;
layout (location = 5) in int model_id;

layout (std140) uniform Camera {
  mat4 projection;
  mat4 view;
};

layout (std430 , binding = 2) readonly buffer ModelData {
  mat4 models[];
};

out VOUT {
  vec3 normal;
} vout;

void main() {
  mat3 normal_mat = mat3(transpose(inverse(view * models[model_id])));
  vout.normal = normalize(vec3(vec4(normal_mat * vnorm , 0.0)));

  gl_Position = view * models[model_id] * vec4(vpos , 1.0);
}
)";

static constexpr std::string_view frag2 = R"(
#version 460 core

out vec4 frag_color;

void main() {
  frag_color = vec4(0.7 , 0.7 , 0.0 , 1.0);
}
)";

static constexpr std::string_view geom = R"(
#version 460 core

layout (triangles) in;
layout (line_strip , max_vertices = 6) out;

layout (std140) uniform Camera {
  mat4 projection;
  mat4 view;
};

in VOUT {
  vec3 normal;
} gin[];

const float magnitude = 0.4f;

// uniform float magnitude;

void GenerateLine(int index) {
  gl_Position = projection * gl_in[index].gl_Position;
  EmitVertex();

  gl_Position = projection * 
                (gl_in[index].gl_Position + vec4(gin[index].normal , 0.0) * magnitude);
  EmitVertex();
  EndPrimitive();
}

void main() {
  GenerateLine(0);
  GenerateLine(1);
  GenerateLine(2);
}

)";

using other::Ref;
using other::NewRef;

using other::Scope;
using other::NewScope;

using other::Shader;
using other::CameraBase;
using other::PerspectiveCamera;
using other::Framebuffer;
using other::Uniform;
using other::UniformBuffer;
using other::ModelSource;
using other::StaticModel;
using other::PipelineSpec;
using other::Pipeline;
using other::RenderPassSpec;
using other::RenderPass;

void CheckGlError(int line);

class TestApp : public other::App {
  public:
    TestApp(other::Engine* engine) 
        : other::App(engine) {}
    virtual ~TestApp() override {}
};

#define CHECK() do { CheckGlError(__LINE__); } while (false)

constexpr static uint32_t win_w = 800;
constexpr static uint32_t win_h = 600;

void LaunchMock(const other::ConfigTable& config) {
  other::IO::Initialize();
  other::EventQueue::Initialize(config);
  other::Renderer::Initialize(config);
  other::UI::Initialize(config , other::Renderer::GetWindow());
  other::ScriptEngine::Initialize(config);
}

void ShutdownMock() {
  other::ScriptEngine::Shutdown();
  other::UI::Shutdown();
  other::Renderer::Shutdown();
  other::EventQueue::Shutdown();
  other::IO::Shutdown();
}

int main(int argc , char* argv[]) {
  try {
    other::CmdLine cmd_line(argc , argv);

    std::string config_path = "C:/Yock/code/OtherEngine/tests/sandbox/sandbox.other"; 
    other::IniFileParser parser(config_path);
    auto config = parser.Parse(); 

    other::Logger::Open(config);
    other::Logger::Instance()->RegisterThread("Sandbox-Thread");

    /// for test reasons
    other::Engine mock_engine = other::Engine::Create(cmd_line , config , config_path);
    {
      Scope<other::App> app_handle = NewScope<TestApp>(&mock_engine);
      mock_engine.LoadApp(app_handle);
    }

    LaunchMock(config);
    OE_DEBUG("Sandbox Launched");

    mock_engine.PushCoreLayer();

    other::ShaderCompiler shader_compilerv1 = other::ShaderCompiler(other::VERTEX_SHADER , std::string(vert1));
    other::ShaderCompiler shader_compilerf1 = other::ShaderCompiler(other::FRAGMENT_SHADER , std::string(frag1));

    other::ShaderIr outputv1 = shader_compilerv1.Compile();
    std::cout << outputv1.source << "\n--------\n";

    other::ShaderIr outputf1 = shader_compilerf1.Compile();
    std::cout << outputf1.source << "\n--------\n";

    {
      Ref<CameraBase> camera = NewRef<PerspectiveCamera>(glm::ivec2{ win_w , win_h });

      RenderPassSpec pass_spec1 = {
        .name = "Test Pass 1" , 
        .tag_col = { 1.f , 0.f , 0.f , 1.f } ,
        .uniforms = {} ,
        .shader = NewRef<Shader>(std::vector<std::string>{ 
          std::string(vert1) , std::string(frag1) 
        }) ,
      };
      
      RenderPassSpec pass_spec2 = {
        .name = "Test Pass 2" , 
        .tag_col = { 1.f , 0.f , 0.f , 1.f } ,
        .uniforms = {} ,
        .shader = NewRef<Shader>(std::vector<std::string>{ 
          std::string(vert2) , std::string(frag2) , 
          std::string(geom) ,
        }) ,
      };

      Ref<RenderPass> pass1 = NewRef<RenderPass>(pass_spec1);
      Ref<RenderPass> pass2 = NewRef<RenderPass>(pass_spec2);
      
      CHECK();

      uint32_t camera_binding_pnt = 0;
      std::vector<Uniform> cam_unis = {
        { "projection" , other::ValueType::MAT4 } ,
        { "view"       , other::ValueType::MAT4 } ,
      };
      
      Ref<UniformBuffer> cam_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt);
      cam_uniforms->BindBase();
      
      uint32_t model_binding_pnt1 = 1;
      uint32_t model_binding_pnt2 = 2;
      std::vector<Uniform> model_unis = {
        { "models" , other::ValueType::MAT4  , 2 } ,
      };

      Ref<UniformBuffer> model_storage1 = NewRef<UniformBuffer>("ModelData" , model_unis , model_binding_pnt1 , 
                                                               other::SHADER_STORAGE , other::DYNAMIC_DRAW);
      model_storage1->BindBase();
      
      Ref<UniformBuffer> model_storage2 = NewRef<UniformBuffer>("ModelData" , model_unis , model_binding_pnt2 , 
                                                               other::SHADER_STORAGE , other::DYNAMIC_DRAW);
      model_storage2->BindBase();
      
      CHECK();

      glm::mat4 model1 = glm::mat4(1.f);
      glm::mat4 model2 = glm::translate(model1 , glm::vec3(0.3f , 0.1f , 0.0f));

      other::AssetHandle model_handle = other::ModelFactory::CreateBox({ 1.f , 1.f , 1.f });
      Ref<StaticModel> model = other::AssetManager::GetAsset<StaticModel>(model_handle);
      Ref<ModelSource> model_source = model->GetModelSource();

      // Scope<SceneRenderer> renderer = NewScope<SceneRenderer>();
      PipelineSpec spec1 = {
        .has_indices = true ,
        .buffer_cap = 4096 * sizeof(float) ,
        .uniform_blocks = {
          { other::FNV("Camera") , cam_uniforms } ,
          { other::FNV("ModelData") , model_storage1 } ,
        } ,
        .framebuffer_spec = {
          .depth_func = other::LESS_EQUAL ,
          .clear_color = { 0.1f , 0.3f , 0.5f , 1.f } ,
          .size = { win_w , win_h } ,
        } ,
        .vertex_layout = model_source->GetLayout() ,
        .debug_name = "Testing Pipeline" , 
      };

      Ref<Pipeline> pipeline1 = NewRef<Pipeline>(spec1);
      pipeline1->SubmitRenderPass(pass1);
      pipeline1->SubmitRenderPass(pass2);

      CHECK();
      
      PipelineSpec spec2 = {
        .has_indices = true ,
        .buffer_cap = 4096 * sizeof(float) ,
        .uniform_blocks = {
          { other::FNV("Camera") , cam_uniforms } ,
          { other::FNV("ModelData") , model_storage2 } ,
        } ,
        .framebuffer_spec = {
          .depth_func = other::LESS_EQUAL ,
          .clear_color = { 0.1f , 0.3f , 0.5f , 1.f } ,
          .size = { win_w , win_h } ,
        } ,
        .vertex_layout = model_source->GetLayout() ,
        .debug_name = "Testing Pipeline 2" , 
      };

      Ref<Pipeline> pipeline2 = NewRef<Pipeline>(spec2);

      CHECK();

      OE_INFO("Running");
      bool running = true;
      while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
          switch (event.type) {
            case SDL_QUIT: running = false; break;
            case SDL_WINDOWEVENT:
              switch (event.window.event) {
                case SDL_WINDOWEVENT_CLOSE: running = false; break; 
                default: break;
              }
            break;
            case SDL_KEYDOWN:
              switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: running = false; break;
                default: break;
              }
            break;
            default:
              break;
          }

          ImGui_ImplSDL2_ProcessEvent(&event);
        }

        /// camera->KeyboardCallback();
        /// camera->MouseCallback();

        model1 = glm::rotate(model1 , glm::radians(5.f) , { 1.f , 1.f , 0.f });

        glClearColor(0.2f , 0.3f , 0.3f , 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        CHECK();

        pipeline1->SubmitStaticModel(model , model1);

        pipeline1->Render(camera);

        other::UI::BeginFrame();

        auto RenderFrame = [](Ref<Framebuffer>& fb , const std::string& title) {
          ImGui::PushID(("##" + title).c_str());
          ImGui::Text("%s" , title.c_str());
          ImGui::SameLine();
          ImGui::Image((void*)(uintptr_t)fb->texture , { win_w , win_h } , ImVec2(0 , 1) , ImVec2(1 , 0));
          ImGui::PopID();
        };

        if (ImGui::Begin("Frames")) {
          auto fb1 = pipeline1->GetOutput();
          // auto fb2 = pipeline2->GetOutput();
          RenderFrame(fb1 , "Pipeline 1");
          // RenderFrame(fb2 , "Pipeline 2");
        } 
        ImGui::End();

        other::UI::EndFrame();
        other::Renderer::GetWindow()->SwapBuffers();
      }
    }

    ShutdownMock();

    OE_INFO("Succesful exit");
    
    other::Logger::Shutdown();
    return 0;
  } catch (const other::IniException& e) {
    std::cout << "caught ini error : " << e.what() << "\n";
  } catch (const other::ShaderException& e) {
    std::cout << "caught shader error : " << e.what() << "\n";
  } catch(const std::exception& e) {
    std::cout << "caught std error : " << e.what() << "\n";
  } catch (...) {
    std::cout << "unknown error" << "\n";
    return 1;
  }
}

void CheckGlError(int line) {
  GLenum err = glGetError();
  while (err != GL_NO_ERROR) {
    switch (err) {
      case GL_INVALID_ENUM: 
        OE_ERROR("OpenGL Error INVALID_ENUM : {}" , line);
      break;
      case GL_INVALID_VALUE:
        OE_ERROR("OpenGL Error INVALID_VALUE : {}" , line);
      break;
      case GL_INVALID_OPERATION:
        OE_ERROR("OpenGL Error INVALID_OPERATION : {}" , line);
      break;
      case GL_STACK_OVERFLOW:
        OE_ERROR("OpenGL Error STACK_OVERFLOW : {}" , line);
      break;
      case GL_STACK_UNDERFLOW:
        OE_ERROR("OpenGL Error STACK_UNDERFLOW : {}" , line);
      break;
      case GL_OUT_OF_MEMORY:
        OE_ERROR("OpenGL Error OUT_OF_MEMORY : {}" , line);
      break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        OE_ERROR("OpenGL Error INVALID_FRAMEBUFFER_OPERATION : {}" , line);
      break;
      case GL_CONTEXT_LOST:
        OE_ERROR("OpenGL Error CONTEXT_LOST : {}" , line);
      break;
      case GL_TABLE_TOO_LARGE:
        OE_ERROR("OpenGL Error TABLE_TOO_LARGE : {}" , line);
      break;
      default: break;
    }
    err = glGetError();
  }
}
