/**
 * \file node_editor_tests/main.cpp
 **/
#include <cstdint>

#include <SDL.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui_node_editor/imgui_node_editor.h>
#include <imgui_node_editor/imgui_node_editor_internal.h>
#include <imflow/ImNodeFlow.h>

#include "core/logger.hpp"
#include "core/errors.hpp"
#include "parsing/ini_parser.hpp"

#include "rendering/window.hpp"

#include "editor/node_editor/node.hpp"

constexpr static uint32_t win_w = 800;
constexpr static uint32_t win_h = 600;

using other::EdgeInput;
using other::EdgeOutput;

struct TestNode : other::Node {
  void AcceptInput(const std::vector<other::ValueList>& values) override {
  }

  std::vector<other::ValueList> RetrieveOutput() override {
    return {};
  }
};

int main() {
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

    ImFlow::ImNodeFlow editor;
    // editor.addNode<TestNode>(ImVec2(0 , 0));

    ax::NodeEditor::Config ne_config;
    ne_config.SettingsFile = "";

    ax::NodeEditor::EditorContext* gui_ctx = ax::NodeEditor::CreateEditor(&ne_config);

    bool first_frame = true;

    struct LinkInfo {
      ax::NodeEditor::LinkId id;
      ax::NodeEditor::PinId in_id;
      ax::NodeEditor::PinId out_id;
    };

    std::vector<LinkInfo> links{};

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

        auto GetUID = []() -> uint64_t {
          static uint64_t uid = 1;
          return uid++;
        };

        ImGui_ImplSDL2_ProcessEvent(&event);
      
        glClearColor(0.2f , 0.3f , 0.3f , 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        /// render here

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(context.window);
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport();

        ax::NodeEditor::SetCurrentEditor(gui_ctx);
        ax::NodeEditor::Begin("Node-Editor");

        ax::NodeEditor::BeginNode(GetUID());
  
        ImGui::Text("Node A");

        ax::NodeEditor::BeginPin(GetUID() , ax::NodeEditor::PinKind::Input);
        ImGui::Text(" -> I ");
        ax::NodeEditor::EndPin();
        
        ax::NodeEditor::BeginPin(GetUID() , ax::NodeEditor::PinKind::Output);
        ImGui::Text(" O -> ");
        ax::NodeEditor::EndPin();

        ax::NodeEditor::EndNode();

        ax::NodeEditor::End();
        ax::NodeEditor::SetCurrentEditor(nullptr);

        // editor.update();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
      
        SDL_GL_MakeCurrent(context.window , context.context);
        SDL_GL_SwapWindow(context.window);
      }
    }

  } catch (const other::IniException& e) {
  } catch (...) {
  }

  return 0;
}
