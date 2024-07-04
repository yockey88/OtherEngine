/**
 * \file sandbox/main.cpp
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
#include <rendering/renderer.hpp>
#include <rendering/rendering_defines.hpp>

#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/ref.hpp"
#include "parsing/ini_parser.hpp"
#include "rendering/window.hpp"
#include "rendering/framebuffer.hpp"

float vertices[] = {
  -0.5f , -0.5f , 0.0f , 1.f , 0.f , 0.f  ,
   0.5f , -0.5f , 0.0f , 0.f , 1.f , 0.f  ,
   0.0f ,  0.5f , 0.0f , 0.f , 0.f , 1.f  
};

static const char* default_vert = R"(
#version 460 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vcol;

out vec4 fcol;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

void main() {
  gl_Position = u_proj * u_view * u_model * vec4(vpos, 1.0);
  fcol = vec4(vcol , 1.0);
}
)";

static const char* default_frag = R"(
#version 460 core

in vec4 fcol;

out vec4 frag_color;

void main() {
  frag_color = fcol;
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
  
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
    
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

    int32_t vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &default_vert, nullptr);
    
    glCompileShader(vert);
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetShaderInfoLog(vert, 512, nullptr, info);
      OE_ERROR("Failed to compile vertex shader: {0}", info);
    }
    
    CHECK();

    int32_t frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &default_frag, nullptr);

    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetShaderInfoLog(frag, 512, nullptr, info);
      OE_ERROR("Failed to compile fragment shader: {0}", info);
    }
    
    CHECK();

    uint32_t shader = 0;
    shader = glCreateProgram();
    glAttachShader(shader, vert);
    glAttachShader(shader, frag);
    glLinkProgram(shader);
    
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
      char info[512];
      glGetProgramInfoLog(shader, 512, nullptr, info);
      OE_ERROR("Failed to link shader program: {0}", info);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    
    CHECK();

    OE_INFO("Shaders Compiled");
    
    uint32_t vao = 0 , vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    
    CHECK();

    OE_INFO("VAO created");
    
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(70.0f), (float)win_w / (float)win_h, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0f);

    int32_t view_loc = glGetUniformLocation(shader , "u_view");
    CHECK();
   
    int32_t proj_loc = glGetUniformLocation(shader , "u_proj");
    CHECK();
    
    int32_t model_loc = glGetUniformLocation(shader , "u_model");
    CHECK();

    glUseProgram(shader);
    CHECK();
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
    CHECK();
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));
    CHECK();
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
    CHECK();
    glUseProgram(0);
    
    CHECK();

    OE_INFO("Uniforms applied");

    // b2World world(b2Vec2(0 , -9.8f));

    // b2BodyDef body_def;
    // body_def.type = b2BodyType::b2_dynamicBody;
    // body_def.position = b2Vec2(0.f , 0.f);
    // body_def.angle = 0.f; 
    // body_def.linearVelocity = b2Vec2(0.f , 0.f);
    // body_def.angularVelocity = 0.f;
    // body_def.angularVelocity = 0.f;
    // body_def.linearDamping = 0.f;
    // body_def.angularDamping = 0.f;
    // body_def.allowSleep = false; 
    // body_def.awake = true; 
    // body_def.fixedRotation = false; 
    // body_def.bullet = false; 
    // body_def.enabled = true;
    // body_def.gravityScale = 1.f;

    // // b2PolygonShape shape;
    // // shape.SetAsBox(1.f , 1.f);

    // // b2FixtureDef fixture_def;
    // // fixture_def.shape = &shape;
    // // fixture_def.density = 1.f;
    // // fixture_def.friction = 0.3f;

    // b2Body* body = world.CreateBody(&body_def);
    // // body->CreateFixture(&fixture_def);

    // if (body != nullptr) {
    //   b2MassData mass_data = body->GetMassData();
    //   mass_data.mass = 1.f;


    //   body->SetMassData(&mass_data);
    // }
      
    // glm::vec3 position = { 0.f , 0.f , 0.f };

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

      fb->BindFrame();
      fb->UnbindFrame();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(context.window);
      ImGui::NewFrame();

      ImGui::ShowDemoWindow();

      if (ImGui::Begin("Test Window")) {
        ImVec2 curr_win_size = ImGui::GetWindowSize();
        float aspect_ratio = 16.f / 9.f;

        ImVec2 size{};
        size.x = fb_spec.size.y * aspect_ratio;
        size.y = fb_spec.size.x / aspect_ratio;

        if (curr_win_size.y >= size.y) {
          size.y = curr_win_size.x;
        } else {
          size.y = curr_win_size.y;
        }

        fb->Resize({ size.x , size.y });

        ImVec2 cursor_pos = {
          (curr_win_size.x - size.x) * 0.5f ,
          ((curr_win_size.y - size.y) * 0.5f) + 7.f
        };
        ImGui::SetCursorPos(cursor_pos);

        ImGui::Image((void*)(uintptr_t)fb->texture , size , ImVec2(0 , 1) , ImVec2(1 , 0)); 
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
    glDeleteProgram(shader);
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
