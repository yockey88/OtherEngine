/**
 * \file gl_sandbox/main.cpp
 **/
#include <iostream>

#include <SDL_events.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#include <SDL_video.h>
#include <glad/glad.h>
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <box2d/box2d.h>
#include <box2d/b2_world.h>
#include <rendering/point_light.hpp>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/ref.hpp"
#include "parsing/ini_parser.hpp"

#include "event/event_queue.hpp"
#include "input/io.hpp"

#include "rendering/window.hpp"
#include "rendering/uniform.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/material.hpp"

#include "sandbox_ui.hpp"

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


static const char* vert1 = R"(
#version 460 core

layout (location = 0) in vec3 vpos;
layout (location = 1) in vec3 vnormal;

layout (std140 , binding = 0) uniform Camera {
  mat4 projection;
  mat4 view;
  vec4 viewpoint;
};

#define MAX_NUM_MODELS 100
layout (std430 , binding = 1) readonly buffer ModelData {
  mat4 models[MAX_NUM_MODELS];
};

out vec3 model_pos;
out vec3 position;
out vec3 normal;

void main() {
  mat4 model = models[gl_InstanceID];
  vec4 world_pos = model * vec4(vpos , 1.0);

  model_pos = vpos;

  mat3 normal_mat = transpose(inverse(mat3(model)));
  normal = normal_mat * vnormal;

  position = world_pos.xyz;
  gl_Position = projection * view * world_pos;
}
)";

static const char* frag1 = R"(
#version 460 core

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo_spec;

in vec3 model_pos;
in vec3 position;
in vec3 normal;

void main() {
  g_position = position;
  g_normal = normalize(normal);
  g_albedo_spec.rgb = model_pos;
  g_albedo_spec.a = 1.f;
}
)";

static const char* vert2 = R"(
#version 460 core

layout (location = 0) in vec3 vpos;
layout (location = 1) in vec2 vtexcoords;

layout (std140 , binding = 0) uniform Camera {
  mat4 projection;
  mat4 view;
  vec4 viewpoint;
};

out vec2 tex_coords;
out vec3 fviewpoint;

void main() {
  fviewpoint = viewpoint.xyz;
  tex_coords = vtexcoords;
  gl_Position = vec4(vpos , 1.0);
}
)";

static const char* frag2 = R"(
#version 460 core

in vec2 tex_coords;
in vec3 fviewpoint;

out vec4 frag_color;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo_spec;

struct PointLight {
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float constant;
  float linear;
  float quadratic;
};

struct DirectionLight {
  vec4 direction;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};

#define MAX_NUM_LIGHTS 100
layout (std430 , binding = 3) readonly buffer Lights {
  vec4 num_lights;
  PointLight point_lights[MAX_NUM_LIGHTS];
  DirectionLight direction_lights[MAX_NUM_LIGHTS];
};

void main() {
  vec3 frag_pos = texture(g_position , tex_coords).rgb;
  vec3 normal = texture(g_normal , tex_coords).rgb;
  vec3 albedo = texture(g_albedo_spec , tex_coords).rgb;
  float specular = texture(g_albedo_spec , tex_coords).a;

  vec3 lighting = albedo * 0.1;
  vec3 view_dir = normalize(fviewpoint - frag_pos);

  for (int i = 0; i < num_lights.y; ++i) {
    // diffuse
    vec3 light_dir = normalize(point_lights[i].position.xyz - frag_pos);
    vec3 diffuse = max(dot(normal , light_dir) , 0.0) * albedo * point_lights[i].ambient.xyz;
    lighting += diffuse;

    // // specular
    // vec3 halfway = normalize(light_dir + view_dir);
    // float s = pow(max(dot(normal , halfway) , 0.0) , 16.0);
    // vec3 spec = point_lights[i].ambient.xyz * s * specular;

    // // attenuation
    // float dist = length(point_lights[i].position.xyz - frag_pos);
    // float attenuation = 1.0 / (1.0 + point_lights[i].linear * dist + point_lights[i].quadratic * dist * dist);

    // diffuse *= attenuation;
    // specular *= attenuation;
    // lighting += diffuse + specular;
  }
  frag_color = vec4(lighting , 1.f);
}
)";

void CheckGlError(int line);

#define CHECK() do { CheckGlError(__LINE__); } while (false)

constexpr static uint32_t win_w = 800;
constexpr static uint32_t win_h = 600;

using other::Ref;
using other::NewRef;

using other::Scope;
using other::NewScope;

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

    int success;

    int32_t vert_1 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_1, 1, &vert1, nullptr);
    
    glCompileShader(vert_1);
    glGetShaderiv(vert_1, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetShaderInfoLog(vert_1, 512, nullptr, info);
      OE_ERROR("Failed to compile vertex shader vert 1: {0}", info);
    }
    
    CHECK();
    
    int32_t vert_2 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_2, 1, &vert2, nullptr);
    
    glCompileShader(vert_2);
    glGetShaderiv(vert_2, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetShaderInfoLog(vert_2, 512, nullptr, info);
      OE_ERROR("Failed to compile vertex shader vert 2: {0}", info);
    }
    
    CHECK();

    int32_t frag_1 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_1, 1, &frag1, nullptr);

    glCompileShader(frag_1);
    glGetShaderiv(frag_1, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[511];
      glGetShaderInfoLog(frag_1, 512, nullptr, info);
      OE_ERROR("Failed to compile fragment shader frag 1: {0}", info);
    }
    
    CHECK();
    
    int32_t frag_2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_2, 1, &frag2, nullptr);

    glCompileShader(frag_2);
    glGetShaderiv(frag_2, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[511];
      glGetShaderInfoLog(frag_2, 512, nullptr, info);
      OE_ERROR("Failed to compile fragment shader frag 2: {0}", info);
    }
    
    CHECK();

    uint32_t shader1 = 0;
    shader1 = glCreateProgram();
    glAttachShader(shader1, vert_1);
    glAttachShader(shader1, frag_1);
    glLinkProgram(shader1);
    
    glGetProgramiv(shader1, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetProgramInfoLog(shader1, 512, nullptr, info);
      OE_ERROR("Failed to link shader program shader 1: {0}", info);
    }

    glDeleteShader(frag_1);
    
    CHECK();

    uint32_t shader2 = 0;
    shader2 = glCreateProgram();
    glAttachShader(shader2, vert_2);
    glAttachShader(shader2, frag_2);
    glLinkProgram(shader2);
    
    glGetProgramiv(shader2, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetProgramInfoLog(shader2, 512, nullptr, info);
      OE_ERROR("Failed to link shader2 program shader 2: {0}", info);
    }

    glDeleteShader(vert_1);
    glDeleteShader(vert_2);
    glDeleteShader(frag_1);
    glDeleteShader(frag_2);

    OE_INFO("Shaders Compiled");

    Quad quad;
    CHECK();
    Cube cube;
    CHECK();

    OE_INFO("VAOs created");

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

    glUseProgram(shader1);
    glUniform1i(glGetUniformLocation(shader1 , "g_position") , 0);
    glUniform1i(glGetUniformLocation(shader1 , "g_normal") , 1);
    glUniform1i(glGetUniformLocation(shader1 , "g_albedo_spec") , 2);
    glUseProgram(shader2);
    glUniform1i(glGetUniformLocation(shader2 , "g_position") , 0);
    glUniform1i(glGetUniformLocation(shader2 , "g_normal") , 1);
    glUniform1i(glGetUniformLocation(shader2 , "g_albedo_spec") , 2);
    glUseProgram(0);

    OE_DEBUG("Uniforms Set");

    glGenFramebuffers(1 , &gbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER , gbuffer);

    glGenTextures(1 , &gbuffer_textures[0]);
    glBindTexture(GL_TEXTURE_2D , gbuffer_textures[0]);
    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGBA16F , win_w , win_h , 0 , GL_RGBA , GL_FLOAT , nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gbuffer_textures[0] , 0);

    // glGenTextures(1 , &gbuffer_textures[1]);
    // glBindTexture(GL_TEXTURE_2D , gbuffer_textures[1]);
    // glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGBA16F , win_w , win_h , 0 , GL_RGBA , GL_FLOAT , nullptr);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D, gbuffer_textures[1] , 0);
    // 
    // glGenTextures(1 , &gbuffer_textures[2]);
    // glBindTexture(GL_TEXTURE_2D , gbuffer_textures[2]);
    // glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGBA , win_w , win_h , 0 , GL_RGBA , GL_FLOAT , nullptr);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gbuffer_textures[2] , 0);

    uint32_t attachments[3] = { 
      GL_COLOR_ATTACHMENT0 ,
      // GL_COLOR_ATTACHMENT1 ,
      // GL_COLOR_ATTACHMENT2 ,
    };
      
    glDrawBuffers(3 , attachments);

    uint32_t render_buffer = 0;
    glGenRenderbuffers(1 , &render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER , render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER , GL_DEPTH_COMPONENT , win_w , win_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_RENDERBUFFER , render_buffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      OE_ERROR("G-Buffer not complete!");
    }

    glm::mat4 display_model = glm::mat4(1.0);

    glBindFramebuffer(GL_FRAMEBUFFER , 0);

    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);
    // glEnable(GL_STENCIL_TEST);
    // glStencilFunc(GL_NOTEQUAL , 1 , 0xFF);
    // glStencilOp(GL_KEEP , GL_KEEP , GL_REPLACE);

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

      model1 = glm::mat4(1.f);
      model1 = glm::rotate(model1 , m1_rotation , { 1.f , 1.f , 1.f });
      m1_rotation += 0.1f;

      /// clear window
      glClearColor(0.f , 0.f , 0.f , 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // GL_STENCIL_BUFFER_BIT);
      
///> GBUFFER RENDER
      glBindFramebuffer(GL_FRAMEBUFFER , gbuffer);

      glDepthMask(GL_TRUE);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glEnable(GL_DEPTH_TEST);
      glDisable(GL_BLEND);

      glUseProgram(shader1);

      model_buffer.ZeroMem();
      model_buffer.BufferData(model1);
      model_buffer.BufferData(model2);
      
      model_uniforms->BindBase();
      model_uniforms->LoadFromBuffer(model_buffer);

      cube.Draw(other::TRIANGLES , 2);
      
      glDepthMask(GL_FALSE);
      glDisable(GL_DEPTH_TEST);
      glBindFramebuffer(GL_FRAMEBUFFER , 0);
/// > END GBUFFER RENDER

/// > LIGHTING PASS
      glUseProgram(shader2);
      
      model_uniforms->BindBase();
      model_uniforms->SetUniform("models" , model_buffer);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D , gbuffer_textures[0]);
      // glActiveTexture(GL_TEXTURE1);
      // glBindTexture(GL_TEXTURE_2D , gbuffer_textures[1]);
      // glActiveTexture(GL_TEXTURE2);
      // glBindTexture(GL_TEXTURE_2D , gbuffer_textures[2]);

      quad.Draw();

      glBindFramebuffer(GL_FRAMEBUFFER , 0);
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
  
// ////// OUTLINE RENDER PASS 
//       float scale = 1.05f;
//       float inv_scale = 1.f / scale;
//       model1 = glm::scale(model1 , { scale , scale , scale });
//       
//       model_buffer.ZeroMem();
//       material_buffer.ZeroMem();
//       model_buffer.BufferData(model1);
//       material_buffer.BufferData(material1);
//       
//   /// > Bind RenderPass
//       glStencilFunc(GL_NOTEQUAL , 0 , 0xFF);
//       glStencilMask(0x00);
//       glDisable(GL_DEPTH_TEST);
//       
//       glUseProgram(shader2);
//   /// > End Bind RenderPass
// 
//   /// > for each mesh in model submissions
//       glBindVertexArray(cube.vao);
// 
//   /// > let render pass set its own uniforms
//       model_uniforms->BindBase();
//       model_uniforms->LoadFromBuffer(model_buffer);
//   /// > end let render pass set its own uniforms 
//       
//     /// draw
//       glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES , 36 , GL_UNSIGNED_INT , (void*)0 , 1 , 0 , 0);
// ////// END OUTLINE RENDER PASS 
// 
// ////// GEOMETRY RENDER PASS 
//       model1 = glm::scale(model1 , { inv_scale , inv_scale , inv_scale });
//       
//       model_buffer.ZeroMem();
//       material_buffer.ZeroMem();
//       model_buffer.BufferData(model1);
//       model_buffer.BufferData(model2);
//       material_buffer.BufferData(material1);
//       material_buffer.BufferData(material2);
// 
//   /// > Bind RenderPass
//       glEnable(GL_DEPTH_TEST);
//       glStencilFunc(GL_ALWAYS , 1 , 0xFF);
//       glStencilMask(0xFF);
// 
//       glUseProgram(shader1);
//       glBindVertexArray(cube.vao);
//   /// > End Bind RenderPass
// 
//   /// > let render pass set its own uniforms
//       material_uniforms->BindBase();
//       material_uniforms->LoadFromBuffer(material_buffer);
//       model_uniforms->BindBase();
//       model_uniforms->LoadFromBuffer(model_buffer);
//   /// > end let render pass set its own uniforms
// 
//     /// > draw
//       glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES , 36 , GL_UNSIGNED_INT , (void*)0 , 2 , 0 , 0);
// ////// END GEOMETRY RENDER PASS 

#if 1
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(context.window);
      ImGui::NewFrame();

      if (ImGui::Begin("GBuffer")) {
        RenderItem(gbuffer_textures[0] , "Position" , ImVec2((float)win_w / 2 , (float)win_h / 2));
        ImGui::SameLine();
        // RenderItem(gbuffer_textures[1] , "Normals" , ImVec2((float)win_w / 2 , (float)win_h / 2));
        // ImGui::SameLine();
        // RenderItem(gbuffer_textures[2] , "Albedo" , ImVec2((float)win_w / 2 , (float)win_h / 2));
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
