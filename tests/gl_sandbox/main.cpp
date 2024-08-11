/**
 * \file gl_sandbox/main.cpp
 **/
#include <SDL_events.h>
#include <event/event_queue.hpp>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <input/io.hpp>
#include <iostream>

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

#include "rendering/rendering_defines.hpp"
#include "rendering/window.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/geometry_pass.hpp"
#include "rendering/material.hpp"

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

  Cube(uint32_t model_idx);
  ~Cube();
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

layout (std430 , binding = 1) readonly buffer ModelData {
  mat4 models[];
};

struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

layout (std430 , binding = 2) readonly buffer MaterialData {
  Material materials[];
};

out vec4 fviewpoint;
out vec3 position;
out vec3 normal;

out Material material;

void main() {
  gl_Position = projection * view * models[gl_InstanceID] * vec4(vpos , 1.0);

  material = materials[gl_InstanceID];

  fviewpoint = viewpoint;
  position = vec3(models[gl_InstanceID] * vec4(vpos , 1.0));
  normal = mat3(transpose(inverse(models[gl_InstanceID]))) * vnormal;
}
)";

static const char* frag1 = R"(
#version 460 core

struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

struct PointLight {
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float constant;
  float linear;
  float quadratic;
};

layout (std430 , binding = 3) readonly buffer LightData {
  int num_point_lights;
  PointLight point_lights[];
};

in vec4 fviewpoint;
in vec3 position;
in vec3 normal;

in Material material;

out vec4 frag_color;

vec3 CalcPointLight(PointLight light , vec3 normal , vec3 frag_pos , vec3 view_dir) {
  vec3 light_dir = normalize(light.position.xyz - frag_pos);
  float diff = max(dot(normal , light_dir) , 0.f);

  vec3 reflect_dir = reflect(-light_dir , normal);
  float spec = pow(max(dot(view_dir , reflect_dir) , 0.0) , material.shininess);

  float distance = length(light.position.xyz - frag_pos);
  float attenuation = 1.f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  vec3 ambient = light.ambient.rgb * material.ambient.rgb; //  * attenuation;
  // vec3 ambient = material.ambient.rgb;
  vec3 diffuse = light.diffuse.rgb * diff * material.diffuse.rgb * attenuation;
  vec3 specular = light.specular.rgb * spec * material.specular.rgb * attenuation;
  
  return (ambient); // + diffuse + specular);
}

void main() {
  // vec3 result = vec3(0.f , 0.f , 0.f);
  // for (int i = 0; i < num_point_lights; ++i) {
  //   result += CalcPointLight(point_lights[i] , normal , position , fviewpoint.xyz); 
  // }
  vec3 result = CalcPointLights(point_lights[0] , normal , position , fviewpoint.xyz);
  frag_color = vec4(result , 1.0);
}
)";

static const char* frag2 = R"(
#version 460 core

struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

struct PointLight {
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float constant;
  float linear;
  float quadratic;
};

in vec4 fviewpoint;
in vec3 position;
in vec3 normal;

in Material material;

out vec4 frag_color;

void main() {
  frag_color = vec4(1.f , 0.f , 0.f , 1.f);
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
      OE_ERROR("Failed to compile vertex shader: {0}", info);
    }
    
    CHECK();

    int32_t frag_1 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_1, 1, &frag1, nullptr);

    glCompileShader(frag_1);
    glGetShaderiv(frag_1, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[511];
      glGetShaderInfoLog(frag_1, 512, nullptr, info);
      OE_ERROR("Failed to compile fragment shader: {0}", info);
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
      OE_ERROR("Failed to link shader program: {0}", info);
    }

    glDeleteShader(frag_1);
    
    CHECK();

    int32_t frag_2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_2, 1, &frag2, nullptr);

    glCompileShader(frag_2);
    glGetShaderiv(frag_2, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[511];
      glGetShaderInfoLog(frag_2, 512, nullptr, info);
      OE_ERROR("Failed to compile fragment shader: {0}", info);
    }
    
    CHECK();

    uint32_t shader2 = 0;
    shader2 = glCreateProgram();
    glAttachShader(shader2, vert_1);
    glAttachShader(shader2, frag_2);
    glLinkProgram(shader2);
    
    glGetProgramiv(shader2, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetProgramInfoLog(shader2, 512, nullptr, info);
      OE_ERROR("Failed to link shader2 program: {0}", info);
    }

    glDeleteShader(vert_1);
    glDeleteShader(frag_2);

    OE_INFO("Shaders Compiled");

    Cube cube1(0);
    CHECK();

    Cube cube2(1);
    CHECK();

    OE_INFO("VAO created");

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
      { "num_point_lights" , other::ValueType::INT32 } ,
      { "point_lights"     , other::ValueType::USER_TYPE , 100 , sizeof(other::PointLight) } ,
    };
    
    Ref<other::UniformBuffer> camera_uniforms = NewRef<other::UniformBuffer>("Camera" , camera_unis , camera_binding_pnt);    
    camera_uniforms->BindBase();

    Ref<other::UniformBuffer> model_uniforms = NewRef<other::UniformBuffer>("ModelData" , model_unis , model_binding_pnt , 
                                                                            other::SHADER_STORAGE);    
    model_uniforms->BindBase();
    
    Ref<other::UniformBuffer> material_uniforms = NewRef<other::UniformBuffer>("MaterialData" , material_unis , material_binding_pnt , 
                                                                                other::SHADER_STORAGE);    
    material_uniforms->BindBase();
    
    Ref<other::UniformBuffer> light_uniforms = NewRef<other::UniformBuffer>("LightData" , light_unis , light_binding_pnt , 
                                                                                other::SHADER_STORAGE);    
    light_uniforms->BindBase();

    other::Buffer camera_buffer;
    other::Buffer model_buffer;
    other::Buffer material_buffer;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL , 1 , 0xFF);
    glStencilOp(GL_KEEP , GL_KEEP , GL_REPLACE);

    OE_INFO("Running");

    bool running = true;
    bool camera_lock = false;
    other::Mouse::LockCursor();

    while (running) {
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

      other::IO::Update();

      /// update camera uniforms
      if (!camera_lock) {
        UpdateCamera(camera);
        proj = camera->ProjectionMatrix();
        view = camera->ViewMatrix();

        camera_uniforms->SetUniform("projection" , camera->ProjectionMatrix());
        camera_uniforms->SetUniform("view" , camera->ViewMatrix());

        glm::vec4 cam_pos = glm::vec4(camera->Position() , 1.f);
        camera_uniforms->SetUniform("viewpoint" , cam_pos);
      }

      light_uniforms->SetUniform("num_point_lights" , 1u);
      light_uniforms->SetUniform("point_lights" , point_light);

      // light_uniforms->BindBase();
      // light_uniforms->SetUniform("num_point_lights" , 1);
      // light_uniforms->SetUniform("point_lights" , point_light);

      model1 = glm::mat4(1.f);
      model1 = glm::rotate(model1 , m1_rotation , { 1.f , 1.f , 1.f });
      m1_rotation += 0.1f;

      /// clear window
      glClearColor(0.2f , 0.3f , 0.3f , 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      
      /// outline pass
      float scale = 1.05f;
      float inv_scale = 1.f / scale;
      model1 = glm::scale(model1 , { scale , scale , scale });
      
      model_buffer.Release();
      model_buffer.BufferData(model1);
      
      glStencilFunc(GL_NOTEQUAL , 0 , 0xFF);
      glStencilMask(0x00);
      glDisable(GL_DEPTH_TEST);
      
      glUseProgram(shader2);
      glBindVertexArray(cube1.vao);
      
      model_uniforms->BindBase();
      model_uniforms->LoadFromBuffer(model_buffer);
      glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES , 36 , GL_UNSIGNED_INT , (void*)0 , 1 , 0 , 0);
      /// end of outline pass
      
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

      /// model pass 
      model1 = glm::scale(model1 , { inv_scale , inv_scale , inv_scale });

      model_buffer.Release();
      material_buffer.Release();

      model_buffer.BufferData(model1);
      material_buffer.BufferData(material1);

      model_buffer.BufferData(model2);
      material_buffer.BufferData(material2);

      glEnable(GL_DEPTH_TEST);
      glStencilFunc(GL_ALWAYS , 1 , 0xFF);
      glStencilMask(0xFF);

      glUseProgram(shader1);
      glBindVertexArray(cube1.vao);

      model_uniforms->BindBase();
      model_uniforms->LoadFromBuffer(model_buffer);
      material_uniforms->BindBase();
      material_uniforms->LoadFromBuffer(material_buffer);
      glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES , 36 , GL_UNSIGNED_INT , (void*)0 , 2 , 0 , 0);
      /// end of model pass

#if 0
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(context.window);
      ImGui::NewFrame();

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
  
Cube::Cube(uint32_t model_idx) {
  vertices = {
    -1.0f / 2.0f, -1.0f / 2.0f,  1.0f / 2.0f ,  -1.f , -1.f ,  1.f , 
     1.0f / 2.0f, -1.0f / 2.0f,  1.0f / 2.0f ,   1.f , -1.f ,  1.f ,
     1.0f / 2.0f,  1.0f / 2.0f,  1.0f / 2.0f ,   1.f ,  1.f ,  1.f ,
    -1.0f / 2.0f,  1.0f / 2.0f,  1.0f / 2.0f ,  -1.f ,  1.f ,  1.f ,
    -1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f ,  -1.f , -1.f , -1.f ,
     1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f ,   1.f , -1.f , -1.f ,
     1.0f / 2.0f,  1.0f / 2.0f, -1.0f / 2.0f ,   1.f ,  1.f , -1.f ,
    -1.0f / 2.0f,  1.0f / 2.0f, -1.0f / 2.0f ,  -1.f ,  1.f , -1.f ,
  }; 
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data() , GL_STATIC_DRAW);
  
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0 , 3 , GL_FLOAT , GL_FALSE , 6 * sizeof(float) , (void*)0);
  glEnableVertexAttribArray(0);
  
  glVertexAttribPointer(1 , 3 , GL_FLOAT , GL_FALSE , 6 * sizeof(float) , (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

Cube::~Cube() {
  glDeleteVertexArrays(1 , &vao);
  glDeleteBuffers(1 , &vbo);
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
