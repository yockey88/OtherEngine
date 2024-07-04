/**
 * \file application\app.cpp
 */
#include "application\app.hpp"

#include "core/logger.hpp"
#include "core/time.hpp"  
#include "core/engine.hpp"
#include "core/config_keys.hpp"
#include "editor/editor_asset_handler.hpp"
#include "input/io.hpp"

#include "event/event_queue.hpp"
#include "event/app_events.hpp"
#include "event/event_handler.hpp"
#include "event/ui_events.hpp"

#include "rendering/renderer.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/ui/ui.hpp"
#include "layers/core_layer.hpp"
#include "layers/debug_layer.hpp"

#include "asset/runtime_asset_handler.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/script_engine.hpp"

#include "physics/phyics_engine.hpp"

namespace other {
namespace {

} /// anonymous namespace

  App::App(Engine* engine) 
      : engine_handle(engine) , cmdline(engine->cmd_line) , config(engine->config) {}

  App::~App() {}
      
  Ref<Project> App::GetProjectContext() {
    return Ref<Project>::Clone(project_metadata);
  }

  void App::Load() {
    OE_DEBUG("Loading application");
    
    OE_DEBUG("Loading Core Systems");

    layer_stack = NewScope<LayerStack>();

    project_metadata = Ref<Project>::Create(cmdline , config);
    if (is_editor) {
      asset_handler = NewRef<EditorAssetHandler>();
      /// register editor console sink
    } else {
      asset_handler = NewRef<RuntimeAssetHandler>();
    }

    scene_manager = NewScope<SceneManager>();
 
    OnLoad();
  }

  void App::Run() {
    Attach();

    CHECKGL();

    time::DeltaTime delta_time;
    delta_time.Start();
    while (!GetEngine()->ShouldQuit()) {
      float dt = delta_time.Get();
      
      // updates mouse/keyboard/any connected controllers
      IO::Update();

      if (Renderer::IsWindowFocused()) {
        DoEarlyUpdate(dt); 
      }

      /// physics step
      
      /// process all events queued from io/early update/physics steps
      EventQueue::Poll(GetEngine() , this);

      /// process any updates that are the result of events here
      
      /// if the window is focuseed we update the application and then the scene
      if (Renderer::IsWindowFocused()) {
        DoUpdate(dt);
        DoLateUpdate(dt);
      }

      /// TODO: redo rendering stage
      ///   ideal workflow:
      ///     RenderPass pass1 = ...
      ///     Renderer::RenderFrame({ camera , framebuffer , pass1 });
      ///
      ///     std::vector<RenderPass> passes = ....
      ///     Renderer::RenderFrame({ camera , framebuffer , passes });
      ///
        
      /**
       * rendering workflow
       *  - for all things that need rendering
       *    - give them the correct renderer
       *    - submit all models
       *
       *  - for all renderers
       *    - execute contstructed pipelines
       **/

      Renderer::BeginFrame();
      DoRender();
      Renderer::EndFrame();
      DoRenderUI();
      Renderer::SwapBuffers();
      
      CHECKGL();
    } 

    Detach();

    OE_DEBUG("Application successfully attached");
  }

  void App::Unload() {
    OE_DEBUG("Unloading application");

    OnUnload();
    
    scene_manager = nullptr;
    asset_handler = nullptr;
    layer_stack = nullptr;

    if (is_editor) {
      OE_DEBUG(" > Editor unloaded");
      return;
    }

    OE_DEBUG(" > Application unloaded");
  }
  
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
    EventHandler event_handler(event);
    event_handler.Handle<UIWindowClosed>([this] (UIWindowClosed& event) -> bool {
      return RemoveUIWindow(event.GetWindowId());
    });

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
    auto itr = ui_windows.find(id);
    if (itr == ui_windows.end()) {
      return false;
    }

    ui_windows[id]->OnDetach();
    ui_windows[id] = nullptr;
    ui_windows.erase(itr);
    return true;
  }

  bool App::RemoveUIWindow(UUID id) {
    auto itr = ui_windows.find(id);
    if (itr == ui_windows.end()) {
      return false;
    }

    ui_windows[id]->OnDetach();
    ui_windows[id] = nullptr;
    ui_windows.erase(itr);
    return true;
  }
  
  void App::LoadScene(const Path& path) {
    if (HasActiveScene() && ActiveScene()->path == path) {
      return;
    }
      
    if (HasActiveScene()) {
      UnloadScene();
    }

    if (!scene_manager->HasScene(path) && !scene_manager->LoadScene(path)) {
      OE_ERROR("Failed to load scene : {}" , path);
      return;
    }
      
    scene_manager->SetAsActive(path);
    const SceneMetadata* scn_metadata = ActiveScene();
    if (scn_metadata == nullptr) {
      OE_ERROR("Failed to load scene : {}" , path);
      return;
    }

    /// propogate scene loading through layers
    for (size_t i = 0; i < layer_stack->Size(); ++i) {
      (*layer_stack)[i]->LoadScene(scn_metadata);
    }

    ScriptEngine::SetSceneContext(scn_metadata->scene);
    PhysicsEngine::SetSceneContext(scn_metadata->scene);
    
    /// alert the client app new scene is loaded 
    OnSceneLoad(ActiveScene());
  }
      
  bool App::HasActiveScene() {
    return scene_manager->ActiveScene() != nullptr;
  }

  SceneMetadata* App::ActiveScene() {
    return scene_manager->ActiveScene();
  }

  void App::UnloadScene() {
    if (ActiveScene() == nullptr) {
      return;
    }

    // scene_manager->SaveActiveScene();
    
    /// alert client app about scene unload
    OnSceneUnload();

    OE_DEBUG("Unloading scene : {}" , ActiveScene()->path);
    scene_manager->UnloadActive();

    /// propogate scene unload through layers
    for (size_t i = 0; i < layer_stack->Size(); ++i) {
      (*layer_stack)[i]->LoadScene(nullptr);
    }

    OE_DEBUG("Scene Unloaded");
  }
      
  bool App::InEditor() const {
    return in_editor;
  }

  void App::Attach() {
    OE_DEBUG("Attaching application");
    OE_DEBUG("Pushing Core Layers");

    {
      Ref<Layer> core_layer = NewRef<CoreLayer>(this);
      PushLayer(core_layer);
    }

    auto debug = config.GetVal<bool>(kProjectSection , kDebugValue);
    if (debug.has_value() && debug.value()) {
      OE_DEBUG("Pushing debug layer");

      Ref<Layer> debug_layer = NewRef<DebugLayer>(this);
      PushLayer(debug_layer);
    }

    
    OnAttach();
  }

  /// TODO: add early update to layers and scene
  void App::DoEarlyUpdate(float dt) {
    EarlyUpdate(dt);  
  }

  void App::DoUpdate(float dt) {
    // update all layers
    for (size_t i = 0; i < layer_stack->Size(); ++i) {
      (*layer_stack)[i]->Update(dt);
    }

    Update(dt);
    
    if (!in_editor) {
      scene_manager->UpdateScene(dt);
    }
    
    /// update the ui windows, closing any that need to be closed 
    auto itr = ui_windows.begin();
    while (itr != ui_windows.end()) {
      itr->second->OnUpdate(dt);
      ++itr;
    }
  }

  void App::DoRender() {
    Render();

    CHECKGL();
    
    for (size_t i = 0; i < layer_stack->Size(); ++i) {
      (*layer_stack)[i]->Render();
    }

    scene_manager->RenderScene();
  }

  void App::DoRenderUI() {
    if (UI::Enabled()) {
      UI::BeginFrame();
      
      RenderUI();
      
      for (size_t i = 0; i < layer_stack->Size(); ++i) {
        (*layer_stack)[i]->UIRender();
      }
      
      for (auto& [id , window] : ui_windows) {
        window->Render();
      }

      scene_manager->RenderSceneUI();

      UI::EndFrame();
    }
  }
      
  void App::DoLateUpdate(float dt) {
    LateUpdate(dt);

    scene_manager->LateUpdateScene(dt); 
  }

  void App::Detach() {
    OE_DEBUG("Detaching application");
    
    OnDetach();

    if (HasActiveScene()) {
      UnloadScene();
    }

    for (auto& [id , window] : ui_windows) {
      window->OnDetach();
      window = nullptr;
    }
    ui_windows.clear();

    for (auto itr = layer_stack->begin(); itr != layer_stack->end(); ++itr) {
      (*itr)->Detach();
    }
    layer_stack->Clear();

    OE_DEBUG("Application detached");
  }

  void App::SetInEditor() {
    in_editor = true;
    is_editor = false;
  }
      
  void App::ReloadScripts() {
    Opt<Path> active_path = std::nullopt;
    if (scene_manager->ActiveScene() != nullptr) {
      active_path = scene_manager->ActiveScene()->path;
      UnloadScene();
    } 
      
    scene_manager->ClearScenes();
    ScriptEngine::ReloadAllScripts();

    OnScriptReload();

    if (active_path.has_value()) {
      LoadScene(active_path.value());
    }
  }

} // namespace other
