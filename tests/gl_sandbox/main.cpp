/**
 * \file gl_sandbox/main.cpp
 **/
#include <SDL_events.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <iostream>

#include <SDL_video.h>
#include <glad/glad.h>
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <box2d/box2d.h>
#include <box2d/b2_world.h>


#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/ref.hpp"
#include "parsing/ini_parser.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/window.hpp"
#include "rendering/framebuffer.hpp"

#define RECT 1

#if RECT
const float vertices[] = {
   0.5f ,  0.5f , 0.0f ,
   0.5f , -0.5f , 0.0f ,
  -0.5f , -0.5f , 0.0f ,
  -0.5f ,  0.5f , 0.0f ,
};

const uint32_t indices[] = {
  0 , 1 , 3 ,
  1 , 2 , 3
};
#else
const float vertices[] = {
  -0.5f , -0.5f , 0.0f ,
   0.5f , -0.5f , 0.0f ,
   0.0f ,  0.5f , 0.0f ,
};
#endif


static const char* vert1 = R"(
#version 460 core

layout(location = 0) in vec3 vpos;

layout (std140) uniform Camera {
  mat4 projection;
  mat4 view;
};

out vec4 fcol;

uniform mat4 u_model;

void main() {
  gl_Position = projection * view * u_model * vec4(vpos , 1.0);
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

static const char* vert2 = R"(
#version 460 core

layout(location = 0) in vec3 vpos;

layout (std140) uniform Camera {
  mat4 projection;
  mat4 view;
};

uniform mat4 u_model;

void main() {
  gl_Position = projection * view * u_model * vec4(vpos , 1.0);
}
)";

static const char* frag2 = R"(
#version 460 core

out vec4 frag_color;

void main() {
  frag_color = vec4(0.7 , 0.7 , 0.7 , 1.0);
}
)";

void CheckGlError(int line);

#define CHECK() do { CheckGlError(__LINE__); } while (false)

constexpr static uint32_t win_w = 800;
constexpr static uint32_t win_h = 600;

int main(int argc , char* argv[]) {
  try {
    other::IniFileParser parser("C:/Yock/code/OtherEngine/tests/sandbox/sandbox.other");
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

    glDeleteShader(vert_1);
    glDeleteShader(frag_1);
    
    CHECK();
    
    int32_t vert_2 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_2, 1, &vert2, nullptr);
    
    glCompileShader(vert_2);
    glGetShaderiv(vert_2, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetShaderInfoLog(vert_2, 512, nullptr, info);
      OE_ERROR("Failed to compile vertex shader: {0}", info);
    }
    
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
    glAttachShader(shader2, vert_2);
    glAttachShader(shader2, frag_2);
    glLinkProgram(shader2);
    
    glGetProgramiv(shader2, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetProgramInfoLog(shader2, 512, nullptr, info);
      OE_ERROR("Failed to link shader2 program: {0}", info);
    }

    glDeleteShader(vert_2);
    glDeleteShader(frag_2);

    OE_INFO("Shaders Compiled");
    
    uint32_t vao = 0 , vbo = 0 , ebo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
#if RECT
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
#endif

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    
    CHECK();

    OE_INFO("VAO created");

    uint32_t cam_idx_1 = glGetUniformBlockIndex(shader1 , "Camera");
    uint32_t cam_idx_2 = glGetUniformBlockIndex(shader2 , "Camera");

    glUniformBlockBinding(shader1 , cam_idx_1 , 0);
    glUniformBlockBinding(shader2 , cam_idx_2 , 0);

    uint32_t camera_ub = 0;
    glGenBuffers(1 , &camera_ub);
    glBindBuffer(GL_UNIFORM_BUFFER , camera_ub);
    glBufferData(GL_UNIFORM_BUFFER , 2 * sizeof(glm::mat4) , nullptr , GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER , 0);

    glBindBufferRange(GL_UNIFORM_BUFFER , 0 , camera_ub , 0 , 2 * sizeof(glm::mat4));
    
    glm::mat4 proj = glm::perspective(glm::radians(70.0f), (float)win_w / (float)win_h, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glBindBuffer(GL_UNIFORM_BUFFER , camera_ub);
    glBufferSubData(GL_UNIFORM_BUFFER , 0 , sizeof(glm::mat4) , glm::value_ptr(proj));
    glBufferSubData(GL_UNIFORM_BUFFER , sizeof(glm::mat4) , sizeof(glm::mat4) , glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER , 0);

    glm::mat4 model1 = glm::mat4(1.0f);
    glm::mat4 model2 = glm::translate(model1 , glm::vec3(0.3f , 0.1f , 0.f));

    int32_t model_loc1 = glGetUniformLocation(shader1 , "u_model");
    int32_t model_loc2 = glGetUniformLocation(shader2 , "u_model");
    CHECK();

    glUseProgram(shader1);
    glUniformMatrix4fv(model_loc1, 1, GL_FALSE, glm::value_ptr(model1));
    CHECK();

    glUseProgram(shader2);
    glUniformMatrix4fv(model_loc2, 1, GL_FALSE, glm::value_ptr(model2));
    CHECK();

    glUseProgram(0);

    OE_INFO("Uniforms applied");

    other::FramebufferSpec fb_spec {
      .size = { win_w , win_h }
    };

    other::Ref<other::Framebuffer> fb = other::NewRef<other::Framebuffer>(fb_spec);

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
              default:
                break;
            }
          break;
          case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
              case SDLK_ESCAPE: running = false; break;
              default:
                break;
            }
          break;
          default:
            break;
        }

        ImGui_ImplSDL2_ProcessEvent(&event);
      }

      glClearColor(0.2f , 0.3f , 0.3f , 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      /// update camera uniforms
      glBindBuffer(GL_UNIFORM_BUFFER , camera_ub);
      glBufferSubData(GL_UNIFORM_BUFFER , 0 , sizeof(glm::mat4) , glm::value_ptr(proj));
      glBufferSubData(GL_UNIFORM_BUFFER , sizeof(glm::mat4) , sizeof(glm::mat4) , glm::value_ptr(view));
      glBindBuffer(GL_UNIFORM_BUFFER , 0);

      fb->BindFrame();


      glUseProgram(shader1);
      glUniformMatrix4fv(model_loc1, 1, GL_FALSE, glm::value_ptr(model1));
      glBindVertexArray(vao);
#if RECT
      glDrawElements(GL_TRIANGLES , 6 , GL_UNSIGNED_INT , 0);
#else
      glDrawArrays(GL_TRIANGLES , 0 , 6);
#endif
      CHECK();

      // glUseProgram(shader2);
      // glUniformMatrix4fv(model_loc2, 1, GL_FALSE, glm::value_ptr(model2));
      // glDrawElements(GL_TRIANGLES , 6 , GL_UNSIGNED_INT , 0);
      // CHECK();

      fb->UnbindFrame();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(context.window);
      ImGui::NewFrame();

      if (ImGui::Begin("Viewport")) {
        ImGui::Text("Frame");
        ImGui::SameLine();
        ImGui::Image((void*)(uintptr_t)fb->texture , { win_w / 2 , win_h / 2} , ImVec2(0 , 1) , ImVec2(1 , 0));
      }
      ImGui::End();

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
  } catch (const other::IniException& e) {
    std::cout << "caught ini error : " << e.what() << "\n";
  } catch(const std::exception& e) {
    std::cout << "caught std error : " << e.what() << "\n";
  } catch (...) {
    std::cout << "unknown error" << "\n";
    return 1;
  }
  return 0;
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
