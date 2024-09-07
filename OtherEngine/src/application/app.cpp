/**
 * \file application\app.cpp
 */
#include "application\app.hpp"

#include "core/logger.hpp"
#include "core/time.hpp"  
#include "core/engine.hpp"
#include "input/io.hpp"

#include "asset/runtime_asset_handler.hpp"

#include "event/event_queue.hpp"
#include "event/app_events.hpp"
#include "event/event_handler.hpp"
#include "event/ui_events.hpp"

#include "rendering/renderer.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/ui/ui.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/script_engine.hpp"

#include "physics/phyics_engine.hpp"
#include <filesystem>
#include <string_view>

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
    OE_DEBUG(" > Loading Core Systems");

    layer_stack = NewScope<LayerStack>();

    project_metadata = Ref<Project>::Create(cmdline , config);
    asset_handler = CreateAssetHandler();

    scene_manager = NewScope<SceneManager>();
 
    OnLoad();

    OE_DEBUG("Application loaded");
  }

  void App::Run() {
    Attach();

    auto& proj_data = project_metadata->GetMetadata();
    if (proj_data.primary_scene.has_value()) {
      LoadSceneByName(*proj_data.primary_scene);
    }

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
      
      /// process all events queued from io/early update/physics steps
      EventQueue::Poll(GetEngine() , this);

      /// process any updates that are the result of events here
      
      /// if the window is focuseed we update the application and then the scene
      if (Renderer::IsWindowFocused()) {
        DoUpdate(dt);
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

      Renderer::GetWindow()->Clear();
      DoRender();
      DoRenderUI();
      Renderer::GetWindow()->SwapBuffers();
      
      CHECKGL();

      // FrameMark;
    } 

    Detach();

    OE_DEBUG("Application successfully detached");
  }

  void App::Unload() {
    OE_DEBUG("Unloading application");

    OnUnload();
    
    scene_manager = nullptr;
    asset_handler = nullptr;
    layer_stack = nullptr;

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
    OE_DEBUG("Popping Layer : {}" , layer->Name());
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
    if (!event->handled) {
      OnEvent(event);
    }

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

  void App::LoadSceneByName(const std::string_view name) {
    auto scene_dir = project_metadata->GetMetadata().assets_dir / "scenes";
    OE_DEBUG("Searching for {} in {}" , name , scene_dir.string());
    for (auto& entry : std::filesystem::directory_iterator(scene_dir)) {
      if (entry.is_regular_file() && entry.path().stem() == name) {
        LoadScene(entry.path());
        return;
      }
    }
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
    
    /// alert the client app new scene is loaded 
    OnSceneLoad(ActiveScene());

    /// propogate scene loading through layers
    for (auto& l : *layer_stack) {
      l->LoadScene(scn_metadata);
    }

    ScriptEngine::SetSceneContext(scn_metadata->scene);
    PhysicsEngine::SetSceneContext(scn_metadata->scene);
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

    /// Do we need to save before we offload
    // scene_manager->SaveActiveScene();
    
    /// alert client app about scene unload
    OnSceneUnload();

    OE_DEBUG("Unloading scene : {}" , ActiveScene()->path);
    scene_manager->UnloadActive();
    
    /// propogate scene loading through layers
    for (auto& l : *layer_stack) {
      l->UnloadScene();
    }

    OE_DEBUG("Scene Unloaded");
  }
  
  void App::ReloadScripts() {
    bool scene_playing = false;
    Opt<Path> active_path = std::nullopt;
    if (scene_manager->ActiveScene() != nullptr) {
      scene_playing = scene_manager->IsPlaying();
      active_path = scene_manager->ActiveScene()->path;
      UnloadScene();
    } 
      
    scene_manager->ClearScenes();
    ScriptEngine::ReloadAllScripts();

    OnScriptReload();

    if (active_path.has_value()) {
      LoadScene(active_path.value());
    }

    if (scene_playing) {
      scene_manager->StartScene();
    }
  }

  void App::Attach() {
    GetProjectContext()->LoadFiles();
    ScriptEngine::LoadProjectModules();
    OnAttach();
  }

  /// TODO: add early update to layers and scene
  void App::DoEarlyUpdate(float dt) {
    EarlyUpdate(dt);  

    layer_stack->InvokeControlledLoop(&Layer::EarlyUpdate , dt);
  }

  void App::DoUpdate(float dt) {
    Update(dt);

    layer_stack->InvokeControlledLoop(&Layer::Update , dt);
    
    LateUpdate(dt);
    
    for (auto& l : *layer_stack) {
      l->LateUpdate(dt);
    }
    
    auto itr = ui_windows.begin();
    while (itr != ui_windows.end()) {
      itr->second->OnUpdate(dt);
      ++itr;
    }
  }

  void App::DoRender() {
    Render();

    CHECKGL();
    
    for (auto& l : *layer_stack) {
      l->Render();
    }

    CHECKGL();
  }

  void App::DoRenderUI() {
    if (UI::Enabled()) {
      UI::BeginFrame();
      
      RenderUI();
      
      layer_stack->InvokeControlledLoop(&Layer::UIRender);
      
      for (auto& [id , window] : ui_windows) {
        window->Render();
      }

      UI::EndFrame();
    }
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

    for (auto& l : *layer_stack) {
      l->Detach();
    }
    layer_stack->Clear();

    ScriptEngine::UnloadProjectModules();

    OE_DEBUG("Application detached");
  }
       
  Ref<AssetHandler> App::CreateAssetHandler() {
    return NewRef<RuntimeAssetHandler>();
  }

} // namespace other
