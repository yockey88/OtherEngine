/**
 * \file gl_sandbox/main.cpp
 **/
#include <iostream>

#include <SDL_events.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#include <SDL_video.h>
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <box2d/box2d.h>
#include <box2d/b2_world.h>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/ref.hpp"
#include "core/filesystem.hpp"
#include "parsing/ini_parser.hpp"

#include "event/event_queue.hpp"
#include "input/io.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/window.hpp"
#include "rendering/vertex.hpp"
#include "rendering/shader.hpp"
#include "rendering/uniform.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/material.hpp"
#include "rendering/point_light.hpp"
#include "rendering/direction_light.hpp"

#include "sandbox_ui.hpp"
#include "gl_helpers.hpp"
#include "shader_embed.hpp"

struct Quad {
  uint32_t vao = 0 , vbo = 0 , ebo = 0;
  std::vector<float> vertices;
  constexpr static uint32_t indices[] = {
    0 , 1 , 2 , 1 , 2 , 3 ,
  };

  Quad();
  ~Quad();

  void Draw();
};

struct Cube {
  uint32_t vao = 0 , vbo = 0 , ebo = 0;
  std::vector<float> vertices;
  constexpr static uint32_t indices[] = {
    0, 1, 2 , 
    2, 3, 0 , 
    1, 5, 6 , 
    6, 2, 1 , 
    7, 6, 5 , 
    5, 4, 7 , 
    4, 0, 3 , 
    3, 7, 4 , 
    4, 5, 1 , 
    1, 0, 4 , 
    3, 2, 6 , 
    6, 7, 3 , 
  };

  Cube();
  ~Cube();

  void Draw(other::DrawMode mode , uint32_t instances = 1);
};

constexpr static uint32_t win_w = 800;
constexpr static uint32_t win_h = 600;

using other::Ref;
using other::NewRef;

using other::Scope;
using other::NewScope;

using other::VertexArray;
using other::Shader;
using other::CameraBase;
using other::PerspectiveCamera;
using other::Framebuffer;

void UpdateCamera(other::Ref<CameraBase>& camera);

int main(int argc , char* argv[]) {
  try {
    other::IniFileParser parser("C:/Yock/code/OtherEngine/tests/gl_sandbox/sandbox.other");
    auto config = parser.Parse(); 

    other::Logger::Open(config);
    other::Logger::Instance()->RegisterThread("Sandbox-Thread");
    
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      throw std::runtime_error("failed to init sdl2");
    } else {
      OE_INFO("SDL initialized successfully");
    }
  
    other::WindowContext context;
    context.window = SDL_CreateWindow("Sandbox" , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED , 
                                      win_w , win_h , SDL_WINDOW_OPENGL);
    if (context.window == nullptr) {
      throw std::runtime_error("Failed to create sdl window");
    } else {
      OE_INFO("Window created successfully");
    }
  
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION , 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION , 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK , SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER , 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE , 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE , 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL , 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS , 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES , 16);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE , 1);

    SDL_GL_SetSwapInterval(1);
  
    context.context = SDL_GL_CreateContext(context.window);
    if (context.context == nullptr) {
      std::string err_str{ SDL_GetError() };
      SDL_DestroyWindow(context.window);
      throw std::runtime_error("failed to create OpenGL context");
    } else {
      OE_INFO("OpenGL context created successfully");
    }
  
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
      SDL_GL_DeleteContext(context.context);
      SDL_DestroyWindow(context.window);
      throw std::runtime_error("Failed to load OpenGL");
    } else {
      OE_INFO("GLAD initialized successfully");
    }
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(context.window, context.context);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    other::IO::Initialize();
    other::EventQueue::Initialize(config);

    OE_INFO("ImGui Initialized successfully"); 

    uint32_t shader1 = other::GetShader(vert1 , frag1);
    uint32_t shader2 = other::GetShader(vert2 , frag2);

    OE_INFO("Shaders Compiled");

    Quad quad;
    CHECK();
    Cube cube;
    CHECK();

    OE_INFO("VAOs created");

    Scope<Framebuffer> frame = NewScope<Framebuffer>(other::FramebufferSpec{});

    std::vector<float> fb_verts = {
       1.f ,  1.f , 1.f , 1.f ,
      -1.f ,  1.f , 0.f , 1.f ,
      -1.f , -1.f , 0.f , 0.f ,
       1.f , -1.f , 1.f , 0.f
    };
    
    std::vector<uint32_t> fb_indices = { 
      0 , 1 , 3 , 
      1 , 2 , 3 
    };
    std::vector<uint32_t> fb_layout = { 
      2 , 2 
    };
      
    Scope<VertexArray> fb_mesh = NewScope<VertexArray>(fb_verts , fb_indices , fb_layout);
    
    const other::Path shader_dir = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders";
    const other::Path fbshader_path = shader_dir / "fbshader.oshader";
    Ref<Shader> fb_shader = other::BuildShader(fbshader_path);

    other::Ref<CameraBase> camera = other::NewRef<PerspectiveCamera>(glm::ivec2{ win_w , win_h });
    camera->SetPosition({ 0.f , 0.f , 3.f });
    glm::mat4 proj = camera->ProjectionMatrix();
    glm::mat4 view = camera->ViewMatrix();

    glm::mat4 model1 =  glm::mat4(1.0f); 
    float m1_rotation = 0.f;

    glm::mat4 model2 =  glm::mat4(1.0f);
    model2 = glm::translate(model2 , glm::vec3(2.f , 0.f , 0.f));

    other::Material material1 = {
      .ambient = { 1.0f , 0.5f , 0.31f , 1.f } ,
      .diffuse = { 0.1f , 0.5f , 0.31f , 1.f } ,
      .specular = { 0.5f , 0.5f , 0.5f , 1.f } ,
      .shininess = 32.f ,
    };
    other::Material material2 = {
      .ambient = { 0.1f , 0.5f , 0.31f , 1.f } ,
      .diffuse = { 1.0f , 0.5f , 0.31f , 1.f } ,
      .specular = { 0.5f , 0.5f , 0.5f , 1.f } ,
      .shininess = 32.f ,
    };

    other::PointLight point_light {
      .position = { 1.2f , 1.0f , 2.0f , 1.f } ,
      .ambient  = { 0.2f , 0.2f , 0.2f , 1.f } ,
      .diffuse  = { 0.5f , 0.5f , 0.5f , 1.f } ,
      .specular = { 1.0f , 1.0f , 1.0f , 1.f } ,
    };
    other::DirectionLight dir_light {
      .direction = { -0.2f , 1.0f , -0.3f , 1.f } ,
      .ambient = { 1.0f , 1.0f , 1.0f , 1.f } ,
      .diffuse = { 1.0f , 1.0f , 1.0f , 1.f } ,
      .specular = { 1.0f , 1.0f , 1.0f , 1.f } ,
    };

    CHECK();

    uint32_t camera_binding_pnt = 0;
    std::vector<other::Uniform> camera_unis = {
      { "projection" , other::ValueType::MAT4 } ,
      { "view"       , other::ValueType::MAT4 } ,
      { "viewpoint"  , other::ValueType::VEC4 } ,
    };

    uint32_t model_binding_pnt = 1;
    std::vector<other::Uniform> model_unis = {
      { "models" , other::ValueType::MAT4 , 100 } ,
    };
    
    uint32_t material_binding_pnt = 2;
    std::vector<other::Uniform> material_unis = {
      { "materials" , other::ValueType::USER_TYPE , 100  , sizeof(other::Material) } ,
    };
    
    uint32_t light_binding_pnt = 3;
    std::vector<other::Uniform> light_unis = {
      { "num_lights" , other::ValueType::VEC4 } ,
      { "point_lights"     , other::ValueType::USER_TYPE , 100 , sizeof(other::PointLight) } ,
      { "direction_lights"     , other::ValueType::USER_TYPE , 100 , sizeof(other::DirectionLight) } ,
    };
    
    Ref<other::UniformBuffer> camera_uniforms = NewRef<other::UniformBuffer>("Camera" , camera_unis , camera_binding_pnt);    
    camera_uniforms->BindBase();
    Ref<other::UniformBuffer> light_uniforms = NewRef<other::UniformBuffer>("Lights" , light_unis , light_binding_pnt , other::SHADER_STORAGE);    
    light_uniforms->BindBase();

    Ref<other::UniformBuffer> material_uniforms = NewRef<other::UniformBuffer>("MaterialData" , material_unis , material_binding_pnt , 
                                                                               other::SHADER_STORAGE);    
    material_uniforms->BindBase();
    Ref<other::UniformBuffer> model_uniforms = NewRef<other::UniformBuffer>("ModelData" , model_unis , model_binding_pnt , 
                                                                            other::SHADER_STORAGE);    
    model_uniforms->BindBase();
    
    other::Buffer model_buffer;
    other::Buffer material_buffer;
    
    uint32_t gbuffer = 0;
    uint32_t gbuffer_textures[4] = {
      0 , 0 , 0 , 0
    };

    glUseProgram(shader2);
    glUniform1i(glGetUniformLocation(shader2 , "g_position") , 0);
    glUniform1i(glGetUniformLocation(shader2 , "g_normal") , 1);
    glUniform1i(glGetUniformLocation(shader2 , "g_albedo_spec") , 2);
    glUseProgram(0);

    OE_DEBUG("Uniforms Set");

    glGenFramebuffers(1 , &gbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER , gbuffer);
    glGenTextures(3 , gbuffer_textures);

    glBindTexture(GL_TEXTURE_2D , gbuffer_textures[0]);
    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGBA16F , win_w , win_h , 0 , GL_RGBA , GL_FLOAT , nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gbuffer_textures[0] , 0);

    glBindTexture(GL_TEXTURE_2D , gbuffer_textures[1]);
    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGBA16F , win_w , win_h , 0 , GL_RGBA , GL_FLOAT , nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D, gbuffer_textures[1] , 0);
    
    glBindTexture(GL_TEXTURE_2D , gbuffer_textures[2]);
    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGBA , win_w , win_h , 0 , GL_RGBA , GL_UNSIGNED_BYTE , nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gbuffer_textures[2] , 0);

    uint32_t attachments[3] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3 , attachments);

    uint32_t render_buffer = 0;
    glGenRenderbuffers(1 , &render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER , render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER , GL_DEPTH_COMPONENT , win_w , win_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_RENDERBUFFER , render_buffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      OE_ERROR("G-Buffer not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER , 0);

    glEnable(GL_DEPTH_TEST);

    OE_INFO("Running");

    bool running = true;
    
    bool camera_lock = true;
    bool force_update = true;
    other::Mouse::FreeCursor();

    while (running) {
      other::IO::Update();
      
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT: running = false; break;
          case SDL_WINDOWEVENT:
            switch (event.window.event) {
              case SDL_WINDOWEVENT_CLOSE: running = false; break; 
              default:
                break;
            }
          break;
          case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
              case SDLK_ESCAPE: running = false; break;
              case SDLK_c: 
                camera_lock = !camera_lock; 
                if (camera_lock) {
                  other::Mouse::FreeCursor();
                } else {
                  other::Mouse::LockCursor();
                }
                break;
              default:
                break;
            }
          break;
          default:
            break;
        }

        ImGui_ImplSDL2_ProcessEvent(&event);
      }

      other::EventQueue::Clear();

      /// update camera uniforms
      if (!camera_lock || force_update) {
        force_update = !force_update;
        UpdateCamera(camera);
        proj = camera->ProjectionMatrix();
        view = camera->ViewMatrix();

        camera_uniforms->SetUniform("projection" , camera->ProjectionMatrix());
        camera_uniforms->SetUniform("view" , camera->ViewMatrix());

        glm::vec4 cam_pos = glm::vec4(camera->Position() , 1.f);
        camera_uniforms->SetUniform("viewpoint" , cam_pos);
      }

      light_uniforms->SetUniform("num_lights" , glm::vec4{ 0 , 1 , 0 , 0 });
      light_uniforms->SetUniform("point_lights" , point_light);
      light_uniforms->SetUniform("direction_lights" , dir_light);

      model1 = glm::mat4(1.f);
      model1 = glm::rotate(model1 , m1_rotation , { 1.f , 1.f , 1.f });
      m1_rotation += 0.1f;

      /// clear window
      glClearColor(0.f , 0.f , 0.f , 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // GL_STENCIL_BUFFER_BIT);
      
///> GBUFFER RENDER
      glBindFramebuffer(GL_FRAMEBUFFER , gbuffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(shader1);

      model_buffer.ZeroMem();
      material_buffer.ZeroMem();

      model_buffer.BufferData(model1);
      material_buffer.BufferData(material1);

      model_buffer.BufferData(model2);
      material_buffer.BufferData(material2);

      model_uniforms->BindBase();
      model_uniforms->LoadFromBuffer(model_buffer);

      material_uniforms->BindBase();
      material_uniforms->LoadFromBuffer(material_buffer);

      cube.Draw(other::TRIANGLES , 2);
      
      glBindFramebuffer(GL_FRAMEBUFFER , 0);
/// > END GBUFFER RENDER

/// > DEBUG
#define DEBUG 0
#if DEBUG
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D , gbuffer_textures[1]);
      fb_shader->SetUniform("screen_tex" , 0);
      fb_mesh->Draw(other::TRIANGLES);
#endif 
/// > END DEBUG

/// > LIGHTING PASS
#if !DEBUG
      glUseProgram(shader2);
      
      model_uniforms->BindBase();
      model_uniforms->SetUniform("models" , model_buffer);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D , gbuffer_textures[0]);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D , gbuffer_textures[1]);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D , gbuffer_textures[2]);

      fb_mesh->Draw(other::TRIANGLES);

      glBindFramebuffer(GL_FRAMEBUFFER , 0);
#endif
/// > END LIGHTING PASS

/// > copy final results default
      // glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer);
      // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
      // // blit to default framebuffer. Note that this may or may not work as the internal formats of 
      // //    both the FBO and default framebuffer have to match.
      // // the internal formats are implementation defined. This works on all of my systems, 
      // //    but if it doesn't on yours you'll likely have to write to th 
      // // depth buffer in another shader stage (or somehow see to match the default 
      // //    framebuffer's internal format with the FBO's internal format).
      // glBlitFramebuffer(0, 0, win_w, win_h, 0, 0, win_w, win_h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
      // glBindFramebuffer(GL_FRAMEBUFFER, 0);

/// set gbuffer data
  
#define UI_ENABLED 1
#if UI_ENABLED
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(context.window);
      ImGui::NewFrame();

      if (ImGui::Begin("GBuffer")) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D , gbuffer_textures[0]);
        RenderItem(gbuffer_textures[0] , "Position" , ImVec2((float)win_w / 2 , (float)win_h / 2));
        ImGui::SameLine();
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D , gbuffer_textures[1]);
        RenderItem(gbuffer_textures[1] , "Normals" , ImVec2((float)win_w / 2 , (float)win_h / 2));
        ImGui::SameLine();
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D , gbuffer_textures[2]);
        RenderItem(gbuffer_textures[2] , "Albedo" , ImVec2((float)win_w / 2 , (float)win_h / 2));
      }
      ImGui::End();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
#endif

      SDL_GL_MakeCurrent(context.window , context.context);
      SDL_GL_SwapWindow(context.window);  
    }

    glDeleteProgram(shader1);
    glDeleteProgram(shader2);

    other::EventQueue::Shutdown();
    other::IO::Shutdown();

    return 1;
  } catch (const other::IniException& e) {
    std::cout << "caught ini error : " << e.what() << "\n";
    other::EventQueue::Shutdown();
    other::IO::Shutdown();
  } catch(const std::exception& e) {
    std::cout << "caught std error : " << e.what() << "\n";
  } catch (...) {
    std::cout << "unknown error" << "\n";
  }

  other::EventQueue::Shutdown();
  other::IO::Shutdown();

  return 1;
}
  
Quad::Quad() {
  vertices = {
   /* (-,+,0) */   -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
   /* (-,-,0) */   -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
   /* (+,+,0) */    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
   /* (+,-,0) */    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
  }; 
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data() , GL_STATIC_DRAW);

  glVertexAttribPointer(0 , 3 , GL_FLOAT , GL_FALSE , 5 * sizeof(float) , (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(2 , 3 , GL_FLOAT , GL_FALSE , 5 * sizeof(float) , (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}

Quad::~Quad() {
  glDeleteVertexArrays(1 , &vao);
  glDeleteBuffers(1 , &vbo);
}

void Quad::Draw() {
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP , 0 , 4);
  glBindVertexArray(0);
}
  
Cube::Cube() {
  vertices = {
   /* (-,-,+) */  -1.0f / 2.0f, -1.0f / 2.0f,  1.0f / 2.0f ,  -1.f , -1.f ,  1.f ,  
   /* (+,-,+) */   1.0f / 2.0f, -1.0f / 2.0f,  1.0f / 2.0f ,   1.f , -1.f ,  1.f , 
   /* (+,+,+) */   1.0f / 2.0f,  1.0f / 2.0f,  1.0f / 2.0f ,   1.f ,  1.f ,  1.f , 
   /* (-,+,+) */  -1.0f / 2.0f,  1.0f / 2.0f,  1.0f / 2.0f ,  -1.f ,  1.f ,  1.f , 
   /* (-,-,-) */  -1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f ,  -1.f , -1.f , -1.f , 
   /* (+,-,-) */   1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f ,   1.f , -1.f , -1.f ,  
   /* (+,+,-) */   1.0f / 2.0f,  1.0f / 2.0f, -1.0f / 2.0f ,   1.f ,  1.f , -1.f , 
   /* (-,+,-) */  -1.0f / 2.0f,  1.0f / 2.0f, -1.0f / 2.0f ,  -1.f ,  1.f , -1.f , 
  }; 
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data() , GL_STATIC_DRAW);
 
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) , indices , GL_STATIC_DRAW);

  glVertexAttribPointer(0 , 3 , GL_FLOAT , GL_FALSE , 6 * sizeof(float) , (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1 , 3 , GL_FLOAT , GL_FALSE , 6 * sizeof(float) , (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

Cube::~Cube() {
  glDeleteVertexArrays(1 , &vao);
  glDeleteBuffers(1 , &vbo);
  glDeleteBuffers(1 , &ebo);
}
  
void Cube::Draw(other::DrawMode mode , uint32_t instances) {
  glBindVertexArray(vao);
  glDrawElementsInstancedBaseVertexBaseInstance(mode , 36 , GL_UNSIGNED_INT , (void*)0 , 2 , 0 , 0);
  glBindVertexArray(0);
}

void UpdateCamera(other::Ref<CameraBase>& camera) {
  if (other::Keyboard::Down(other::Keyboard::Key::OE_W)) {
    camera->MoveForward();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_S)) {
    camera->MoveBackward();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_A)) {
    camera->MoveLeft();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_D)) {
    camera->MoveRight();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_SPACE)) {
    camera->MoveUp();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_LSHIFT)) {
    camera->MoveDown();
  }

  glm::vec2 win_size = { win_w , win_h };
  glm::ivec2 mouse_pos = other::Mouse::GetPos();

  SDL_WarpMouseInWindow(SDL_GetMouseFocus() , win_size.x / 2 , win_size.y / 2);

  camera->SetLastMouse(camera->Mouse());
  camera->SetMousePos(mouse_pos);
  camera->SetDeltaMouse({ 
    camera->Mouse().x - camera->LastMouse().x ,
    camera->LastMouse().y - camera->Mouse().y
  });

    glm::ivec2 rel_pos = other::Mouse::GetRelPos();

    camera->SetYaw(camera->Yaw() + (rel_pos.x * camera->Sensitivity()));
    camera->SetPitch(camera->Pitch() - (rel_pos.y * camera->Sensitivity()));

    if (camera->ConstrainPitch()) {
      if (camera->Pitch() > 89.0f) {
        camera->SetPitch(89.0f);
      }

      if (camera->Pitch() < -89.0f) {
        camera->SetPitch(-89.0f);
      }
    }

    camera->UpdateCoordinateFrame();
    camera->CalculateMatrix();
}
