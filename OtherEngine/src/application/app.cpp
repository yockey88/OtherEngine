/**
 * \file application\app.cpp
 */
#include "application\app.hpp"

#include "core/logger.hpp"
#include "core/time.hpp"  
#include "core/engine.hpp"
#include "input/io.hpp"
#include "event/event_queue.hpp"
#include "event/app_events.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "layers/core_layer.hpp"
#include "layers/debug_layer.hpp"

namespace other {

  void App::PushLayer(Ref<Layer>&  layer) {
    layer->Attach();
    layer_stack->PushLayer(layer);

    EventQueue::PushEvent<AppLayerEvent>(LayerEventType::LAYER_PUSH , layer->GetUUID() , layer->Name());
  }

  void App::PushOverlay(Ref<Layer>&  overlay) {
    overlay->Attach();
    layer_stack->PushOverlay(overlay);

    EventQueue::PushEvent<AppLayerEvent>(LayerEventType::OVERLAY_PUSH , overlay->GetUUID() , overlay->Name());
  }

  void App::PopLayer(Ref<Layer>&  layer) {
    layer->Detach();
    layer_stack->PopLayer(layer);
    EventQueue::PushEvent<AppLayerEvent>(LayerEventType::LAYER_POP , layer->GetUUID() , layer->Name());
  }

  void App::PopLayer() {
    if (layer_stack->Empty()) {
      OE_WARN("Attempting to pop a layer from an empty layer stack");
      return;
    }

    layer_stack->Top()->Detach();
    EventQueue::PushEvent<AppLayerEvent>(LayerEventType::LAYER_POP , layer_stack->Top()->GetUUID() , layer_stack->Top()->Name());
    layer_stack->PopLayer();
  }

  void App::PopOverlay(Ref<Layer>&  overlay) {
    overlay->Detach();
    layer_stack->PopOverlay(overlay);

    EventQueue::PushEvent<AppLayerEvent>(LayerEventType::OVERLAY_POP , layer_stack->Top()->GetUUID() , overlay->Name());
  }

  void App::ProcessEvent(Event* event) {
    for (auto& window : ui_windows) {
      window.second->OnEvent(event);
    }

    for (auto itr = layer_stack->end(); itr != layer_stack->begin();) {
      (*--itr)->ProcessEvent(event);
    
      if (event->handled) {
        itr = layer_stack->begin();
      }
    }

    if (!event->handled) {
      OnEvent(event);
    }
  }

  App::App(Engine* engine) 
    :  cmdline(engine->cmd_line) , config(engine->config) , engine_handle(engine) {
  }

  App::~App() {}

  void App::LoadMetadata(const Ref<Project>& metadata) {
    project_metadata = metadata;
  }

  void App::OnLoad() {
    layer_stack = NewScope<LayerStack>();
    asset_handler = NewScope<AssetHandler>();

    scene_manager = NewScope<SceneManager>();

    {
      Ref<Layer> core_layer = NewRef<CoreLayer>(this);
      PushLayer(core_layer);
    }

    auto debug = config.GetVal<bool>("PROJECT" , "DEBUG");
    if (debug.has_value() && debug.value()) {
      Ref<Layer> debug_layer = NewRef<DebugLayer>(this);
      PushLayer(debug_layer);
    }
  }

  void App::Run() {
    Attach();

    ///> TODO: experiment and see if we want to use delta time or frame rate enforcer
    time::FrameRateEnforcer<60> frame_rate_enforcer;
    time::DeltaTime delta_time;
    delta_time.Start();
    while (!GetEngine()->exit_code.has_value()) {
      float dt = delta_time.Get();
      // float dt = frame_rate_enforcer.TimeStep();

      // updates mouse/keyboard/any connected controllers
      IO::Update();
      EventQueue::Poll(GetEngine() , this);
      
      // update all layers
      for (size_t i = 0; i < layer_stack->Size(); ++i) {
        (*layer_stack)[i]->Update(dt);
      }

      DoUpdate(dt);

      UpdateSceneContext(dt);

      Renderer::BeginFrame();

      // if (scene_context != nullptr) {
      //   scene_context->Render();
      // }

      DoRender();

      // if (GetEngine()->window->FramebufferAttached()) {
      //   GetEngine()->window->DrawFramebuffer();
      // }

      if (UI::Enabled()) {
        UI::BeginFrame();

        // client ui render
        DoRenderUI();

        // if (scene_context != nullptr) {
        //   scene_context->RenderUI();
        // }

        { // render all layers
          for (size_t i = 0; i < layer_stack->Size(); ++i) {
            (*layer_stack)[i]->UIRender();
          }
        }

        UI::EndFrame();
      }

      Renderer::EndFrame();

      frame_rate_enforcer.Enforce();
    } 

    Detach();
  }

  void App::OnUnload() {
    asset_handler = nullptr;

    layer_stack->Clear(); 
    layer_stack = nullptr;
  }

  static Ref<UIWindow> null_window = nullptr;

  Ref<UIWindow>& App::GetUIWindow(const std::string& name) {
    UUID id = FNV(name);
    if (ui_windows.find(id) == ui_windows.end()) {
      return null_window;
    }

    return ui_windows[id];
  }

  Ref<UIWindow>& App::GetUIWindow(UUID id) {
    if (ui_windows.find(id) == ui_windows.end()) {
      return null_window;
    }

    return ui_windows[id];
  }

  UUID App::PushUIWindow(const std::string& name , Ref<UIWindow> window) {
    UUID id = FNV(name);
    if (ui_windows.find(id) != ui_windows.end()) {
      OE_WARN("UIWindow with name: {} already exists, overwriting", name);
    }

    window->OnAttach();
    ui_windows[id] = window;

    return id;
  }

  UUID App::PushUIWindow(Ref<UIWindow> window) {
    UUID id = FNV(window->Title());
    if (ui_windows.find(id) != ui_windows.end()) {
      OE_WARN("UIWindow with name: {} already exists, overwriting", window->Title());
      return 0;
    }

    window->OnAttach();
    ui_windows[id] = window;
    return id;
  }

  bool App::RemoveUIWindow(const std::string& name) {
    UUID id = FNV(name);
    if (ui_windows.find(id) == ui_windows.end()) {
      return false;
    }

    ui_windows[id]->OnDetach();
    ui_windows[id] = nullptr;
    ui_windows.erase(id);
    return true;
  }

  bool App::RemoveUIWindow(UUID id) {
    if (ui_windows.find(id) == ui_windows.end()) {
      return false;
    }

    ui_windows[id]->OnDetach();
    ui_windows[id] = nullptr;
    ui_windows.erase(id);
    return true;
  }
  
  void App::LoadScene(const Path& path) {
    if (scene_manager->HasScene(path)) {
      scene_manager->SetAsActive(path);
    } else {
      if (!scene_manager->LoadScene(path)) {
        OE_ERROR("Failed to load scene : {}" , path);
        return;
      }
      UnloadScene();
      
      scene_manager->SetAsActive(path);
      if (scene_manager->ActiveScene() == nullptr) {
        OE_ERROR("Failed to load scene : {}" , path);
        return;
      }

      /// propogate scene loading through layers
      for (size_t i = 0; i < layer_stack->Size(); ++i) {
        (*layer_stack)[i]->LoadScene(scene_manager->ActiveScene());
      }
      
      /// alert the client app new scene is loaded 
      OnSceneLoad(scene_manager->ActiveScene());
    }
  }

  Ref<Scene> App::ActiveScene() {
    return scene_manager->ActiveScene()->scene;
  }

  void App::UnloadScene() {
    if (scene_manager->ActiveScene() == nullptr) {
      return;
    }

    OE_DEBUG("Unloading scene : {}" , scene_manager->ActiveScene()->path);
    scene_manager->UnloadActive();
  }

  void App::Attach() {
    OnAttach();
  }

  void App::DoUpdate(float dt) {
    auto itr = ui_windows.begin();
    while (itr != ui_windows.end()) {
      if (itr->second == nullptr) {
        itr = ui_windows.erase(itr);
      } else if (!itr->second->IsOpen()) {
        itr->second->OnDetach();
        itr->second = nullptr;
        itr = ui_windows.erase(itr);
      } else {
        itr->second->OnUpdate(dt);
        ++itr;
      }
    }
    Update(dt);
  }

  void App::DoRender() {
    Render();
    
    if (scene_manager->ActiveScene() != nullptr) {
      //scene_manager->ActiveScene()->Render();
    }

    for (size_t i = 0; i < layer_stack->Size(); ++i) {
      (*layer_stack)[i]->Render();
    }
  }

  void App::DoRenderUI() {
    RenderUI();
    for (auto& [id , window] : ui_windows) {
      window->Render();
    }
  }

  void App::Detach() {
    OnDetach();

    for (auto& [id , window] : ui_windows) {
      window->OnDetach();
      window = nullptr;
    }
    ui_windows.clear();
  }
      
  void App::UpdateSceneContext(float dt) {
    scene_manager->UpdateScene(dt);
  }

} // namespace other
