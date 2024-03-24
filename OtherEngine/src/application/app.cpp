/**
 * \file application\app.cpp
 */
#include "application\app.hpp"

#include <filesystem>

#include "core/logger.hpp"
#include "core/time.hpp"  
#include "core/engine.hpp"
#include "input/io.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "layers/core_layer.hpp"
#include "layers/debug_layer.hpp"

namespace other {

  void App::PushLayer(Ref<Layer>&  layer) {
    layer->OnAttach();
    layer_stack->PushLayer(layer);
  }

  void App::PushOverlay(Ref<Layer>&  overlay) {
    overlay->OnAttach();
    layer_stack->PushOverlay(overlay);
  }

  void App::PopLayer(Ref<Layer>&  layer) {
    layer->OnDetach();
    layer_stack->PopLayer(layer);
  }

  void App::PopOverlay(Ref<Layer>&  overlay) {
    overlay->OnDetach();
    layer_stack->PopOverlay(overlay);
  }

  void App::ProcessEvent(Event* event) {
    for (auto itr = layer_stack->end(); itr != layer_stack->begin();) {
      (*--itr)->OnEvent(event);
    
      if (event->handled) {
        itr = layer_stack->begin();
      }
    }

    if (!event->handled) {
      OnEvent(event);
    }
  }
  
  void App::LoadScene(const std::string& scene_file) {
    if (!std::filesystem::exists(scene_file)) {
      OE_ERROR("Scene file does not exist: {0}" , scene_file);
      return;
    }
  }

  App::App(Engine* engine , const ConfigTable& cfg) {
    engine_handle = engine;
    config = cfg;
  }

  App::~App() {
    for (auto& layer : *layer_stack) {
      layer->OnDetach();
    }
  }

  void App::OnLoad() {
    layer_stack = NewScope<LayerStack>();
    asset_handler = NewScope<AssetHandler>();

    {
      Ref<Layer> core_layer = NewRef<CoreLayer>(this);
      PushLayer(core_layer);
    }

    auto debug = config.GetVal<bool>("APP" , "DEBUG");
    if (debug.has_value() && debug.value()) {
      Ref<Layer> debug_layer = NewRef<DebugLayer>(this);
      PushOverlay(debug_layer);
    }

    // PushLayer(NewScope<BaseLayer>(this));
    // if (app_data->app_config.debug_mode) {
    //   PushOverlay(NewScope<DebugLayer>(this));
    // }

    // GetEngine()->script_engine->SetAppContext(this);
    // LanguageModule* module = GetEngine()->script_engine->GetModule("Mono");
    // if (module == nullptr) {
    //   LogError(Logger::LogTarget::ALL_LOGS , "Mono module not found");
    // } else {
    //   LogInfo(Logger::LogTarget::ALL_LOGS , "Loading EngineY-ScriptCore module");
#ifdef ENGINEY_DEBUG
    // std::string module_path = "bin/Debug/EngineY-ScriptCore/EngineY-ScriptCore.dll";
#else
    //  std::string module_path = "bin/Release/EngineY-ScriptCore/EngineY-ScriptCore.dll";
#endif
    //  LogDebug(Logger::LogTarget::CONSOLE , "Loading module from path: {}" , module_path);
    //  module->LoadScriptModule({ "EngineY-ScriptCore" , { module_path } });
    // }
    
    OnAttach();
  }

  void App::Run() {
    OE_DEBUG("Running app, initializing...");
    Initialize();

    ///> TODO: experiment and see if we want to use delta time or frame rate enforcer
    // time::DeltaTime delta_time;
    // delta_time.Start();
    OE_DEBUG("App initialized, running main loop, enforcing 60 fps");
    time::FrameRateEnforcer<60> frame_rate_enforcer;
    while (!GetEngine()->exit_code.has_value()) {
      // float dt = delta_time.DeltaTime();
      float dt = frame_rate_enforcer.TimeStep();

      // updates mouse/keyboard/any connected controllers
      IO::Update();
      EventQueue::Poll(GetEngine() , this);
      
      // update all layers
      for (size_t i = 0; i < layer_stack->Size(); ++i) {
        (*layer_stack)[i]->OnUpdate(dt);
      }

      // client update
      Update(dt);

      // if (scene_context != nullptr) {
      //   scene_context->Update(dt);
      // }

      // GetEngine()->ui->UpdateWindows(dt);

      Renderer::BeginFrame();

      // Render(); // render all layers
      // if (scene_context != nullptr) {
      //   scene_context->Render();
      // }

      for (size_t i = 0; i < layer_stack->Size(); ++i) {
        (*layer_stack)[i]->OnRender();
      }

      // if (GetEngine()->window->FramebufferAttached()) {
      //   GetEngine()->window->DrawFramebuffer();
      // }

      if (UI::Enabled()) {
        UI::BeginFrame();

        // client render
        RenderUI();

        // if (scene_context != nullptr) {
        //   scene_context->RenderUI();
        // }

        { // render all layers
          for (size_t i = 0; i < layer_stack->Size(); ++i) {
            (*layer_stack)[i]->OnUIRender();
          }
        }

        UI::EndFrame();
      }

      Renderer::EndFrame();

      frame_rate_enforcer.Enforce();
    } 

    OE_DEBUG("Main loop exited, shutting down...");
    Shutdown();
  }

  void App::OnUnload() {
    OnDetach();

    // LanguageModule* module = GetEngine()->script_engine->GetModule("Mono");
    // module->UnloadScriptModule("EngineY-ScriptCore");
  }

} // namespace other
