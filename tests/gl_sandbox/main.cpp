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
#include <rendering/camera_base.hpp>


#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/ref.hpp"
#include "parsing/ini_parser.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/window.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/perspective_camera.hpp"

#define RECT 0

#if RECT
const float vertices[] = {
   0.5f ,  0.5f , 0.0f , 0.f ,
   0.5f , -0.5f , 0.0f , 0.f ,
  -0.5f , -0.5f , 0.0f , 0.f ,
  -0.5f ,  0.5f , 0.0f , 0.f ,
};

const uint32_t indices[] = {
  0 , 1 , 3 ,
  1 , 2 , 3
};
#else
const float vertices[] = {
  -1.0f / 2.0f, -1.0f / 2.0f,  1.0f / 2.0f , 0.f ,
   1.0f / 2.0f, -1.0f / 2.0f,  1.0f / 2.0f , 0.f ,
   1.0f / 2.0f,  1.0f / 2.0f,  1.0f / 2.0f , 0.f , 
  -1.0f / 2.0f,  1.0f / 2.0f,  1.0f / 2.0f , 0.f , 
  -1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f , 0.f , 
   1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f , 0.f , 
   1.0f / 2.0f,  1.0f / 2.0f, -1.0f / 2.0f , 0.f , 
  -1.0f / 2.0f,  1.0f / 2.0f, -1.0f / 2.0f , 0.f , 
};

const uint32_t indices[] = {
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

#endif


static const char* vert1 = R"(
#version 460 core

layout (location = 0) in vec3 vpos;
layout (location = 1) in int model_id; 

layout (std140) uniform Camera {
  mat4 projection;
  mat4 view;
};

layout (std430) buffer ModelData {
  mat4 models[];
};

out vec4 fcol;

void main() {
  gl_Position = projection * view * models[model_id] * vec4(vpos , 1.0);
  fcol = vec4(vpos , 1.0);
}
)";

static const char* frag1 = R"(
#version 460 core

in vec4 fcol;
out vec4 frag_color;

void main() {
  frag_color = fcol;
}
)";

static const char* frag2 = R"(
#version 460 core

out vec4 frag_color;

void main() {
  frag_color = vec4(1.f , 0.f , 0.f , 1.f);
}
)";

void CheckGlError(int line);

#define CHECK() do { CheckGlError(__LINE__); } while (false)

constexpr static uint32_t win_w = 800;
constexpr static uint32_t win_h = 600;

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
    
    uint32_t vao = 0 , vbo = 0 , ebo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0 , 3 , GL_FLOAT , GL_FALSE , 4 * sizeof(float) , (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1 , 1 , GL_FLOAT , GL_FALSE , 4 * sizeof(float) , (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    
    CHECK();

    OE_INFO("VAO created");

    uint32_t cam_idx_1 = glGetUniformBlockIndex(shader1 , "Camera");
    uint32_t cam_idx_2 = glGetUniformBlockIndex(shader2 , "Camera");

    glUniformBlockBinding(shader1 , cam_idx_1 , 0);
    glUniformBlockBinding(shader2 , cam_idx_2 , 0);
    
    uint32_t model_idx_1 = glGetProgramResourceIndex(shader1 , GL_SHADER_STORAGE_BLOCK, "ModelData");
    uint32_t model_idx_2 = glGetProgramResourceIndex(shader2 , GL_SHADER_STORAGE_BLOCK, "ModelData");

    glShaderStorageBlockBinding(shader1 , model_idx_1 , 1);
    glShaderStorageBlockBinding(shader2 , model_idx_2 , 1);

    uint32_t camera_ub = 0;
    glGenBuffers(1 , &camera_ub);
    glBindBuffer(GL_UNIFORM_BUFFER , camera_ub);
    glBufferData(GL_UNIFORM_BUFFER , 2 * sizeof(glm::mat4) , nullptr , GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER , 0);

    glBindBufferRange(GL_UNIFORM_BUFFER , 0 , camera_ub , 0 , 2 * sizeof(glm::mat4));
    
    other::Ref<CameraBase> camera = other::NewRef<PerspectiveCamera>(glm::ivec2{ win_w , win_h });
    camera->SetPosition({ 0.f , 0.f , 3.f });
    glm::mat4 proj = camera->ProjectionMatrix();
    glm::mat4 view = camera->ViewMatrix();

    glBindBuffer(GL_UNIFORM_BUFFER , camera_ub);
    glBufferSubData(GL_UNIFORM_BUFFER , 0 , sizeof(glm::mat4) , glm::value_ptr(proj));
    glBufferSubData(GL_UNIFORM_BUFFER , sizeof(glm::mat4) , sizeof(glm::mat4) , glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER , 0);

    glm::mat4 model1 =  glm::mat4(1.0f); 
    glm::mat4 model2 =  glm::mat4(1.f);
    model2 = glm::translate(model1 , glm::vec3(0.3f , 0.1f , 0.f));

    uint32_t model_ssbo = 0;
    glGenBuffers(1 , &model_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER , model_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER , 2 * sizeof(glm::mat4) , nullptr , GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER , 0);

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER , 1 , model_ssbo , 0 , 2 * sizeof(glm::mat4));

    glBindBuffer(GL_SHADER_STORAGE_BUFFER , model_ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER , 0 , sizeof(glm::mat4) , glm::value_ptr(model1));
    glBufferSubData(GL_SHADER_STORAGE_BUFFER , sizeof(glm::mat4) , sizeof(glm::mat4) , glm::value_ptr(model2));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER , 0);

    CHECK();

    OE_INFO("Uniforms applied");

    other::FramebufferSpec fb_spec {
      .size = { win_w , win_h }
    };
    
    other::FramebufferSpec fb2_spec {
      .clear_color = { 0.1f , 1.f , 1.f , 1.f } ,
      .size = { win_w , win_h } ,
    };

    other::Ref<Framebuffer> fb = other::NewRef<Framebuffer>(fb_spec);
    other::Ref<Framebuffer> fb2 = other::NewRef<Framebuffer>(fb2_spec);

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

      if (!camera_lock) {
        UpdateCamera(camera);
        proj = camera->ProjectionMatrix();
        view = camera->ViewMatrix();
      }

      // model1 = glm::rotate(model1 , 0.1f , { 1.f , 1.f , 1.f });

      /// clear window
      glClearColor(0.2f , 0.3f , 0.3f , 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      /// update camera uniforms
      glBindBuffer(GL_UNIFORM_BUFFER , camera_ub);
      glBufferSubData(GL_UNIFORM_BUFFER , 0 , sizeof(glm::mat4) , glm::value_ptr(proj));
      glBufferSubData(GL_UNIFORM_BUFFER , sizeof(glm::mat4) , sizeof(glm::mat4) , glm::value_ptr(view));
      glBindBuffer(GL_UNIFORM_BUFFER , 0);
      
      /// outline pass
      float scale = 1.03f;
      model1 = glm::mat4(1.f);
      model1 = glm::scale(model1 , { scale , scale , scale });
      
      glBindBuffer(GL_SHADER_STORAGE_BUFFER , model_ssbo);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER , 0 , sizeof(glm::mat4) , glm::value_ptr(model1));
      glBindBuffer(GL_SHADER_STORAGE_BUFFER , 0);

      glStencilFunc(GL_NOTEQUAL , 0 , 0xFF);
      glStencilMask(0x00);
      glDisable(GL_DEPTH_TEST);

      glUseProgram(shader2);
      glBindVertexArray(vao);
      glDrawElements(GL_TRIANGLES , 
#if RECT
        6 , 
#else
        36 ,
#endif
      GL_UNSIGNED_INT , 0);
      /// end of outline pass

      /// model pass 
      model1 = glm::mat4(1.f);

      glBindBuffer(GL_SHADER_STORAGE_BUFFER , model_ssbo);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER , 0 , sizeof(glm::mat4) , glm::value_ptr(model1));
      glBufferSubData(GL_SHADER_STORAGE_BUFFER , sizeof(glm::mat4) , sizeof(glm::mat4) , glm::value_ptr(model2));
      glBindBuffer(GL_SHADER_STORAGE_BUFFER , 0);

      glEnable(GL_DEPTH_TEST);
      glStencilFunc(GL_ALWAYS , 1 , 0xFF);
      glStencilMask(0xFF);

      glUseProgram(shader1);
      glDrawElements(GL_TRIANGLES , 
#if RECT
        6 , 
#else
        36 ,
#endif
      GL_UNSIGNED_INT , 0);
      /// end of model pass


      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(context.window);
      ImGui::NewFrame();

      // if (ImGui::Begin("Viewport")) {
      //   ImGui::Text("Frame");
      //   ImGui::SameLine();
      //   // ImGui::Image((void*)(uintptr_t)fb->texture , { (float)win_w / 2 , (float)win_h / 2} , ImVec2(0 , 1) , ImVec2(1 , 0));
      //   // ImGui::Image((void*)(uintptr_t)fb2->texture , { (float)win_w / 2 , (float)win_h / 2} , ImVec2(0 , 1) , ImVec2(1 , 0));
      // }
      // ImGui::End();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();

      SDL_GL_MakeCurrent(context.window , context.context);
      SDL_GL_SwapWindow(context.window);
    }


    glDeleteVertexArrays(1 , &vao);
    glDeleteBuffers(1 , &vbo);
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
