/**
 * \file application\app.cpp
 */
#include "application\app.hpp"

#include "core/logger.hpp"
#include "core/time.hpp"  
#include "core/engine.hpp"
#include "core/config_keys.hpp"
#include "input/io.hpp"

#include "event/event_queue.hpp"
#include "event/app_events.hpp"
#include "event/event_handler.hpp"

#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "layers/core_layer.hpp"
#include "layers/debug_layer.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  App::App(Engine* engine) 
    :  cmdline(engine->cmd_line) , config(engine->config) , engine_handle(engine) {
  }

  App::~App() {}
      
  Ref<Project> App::GetProjectContext() {
    return Ref<Project>::Clone(project_metadata);
  }

  void App::Load() {
    OE_DEBUG("Loading application");

    project_metadata = Ref<Project>::Create(cmdline , config);

    layer_stack = NewScope<LayerStack>();
    asset_handler = NewScope<AssetHandler>();

    scene_manager = NewScope<SceneManager>();

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
     
    /// return because the editor will have already loaded all of this stuff already
    if (in_editor) {
      return;
    }
    
    ScriptEngine::SetAppContext(this);
    ScriptEngine::Initialize(GetEngine() , config);

    OE_DEBUG("Loading C# script modules");
    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(LanguageModuleType::CS_MODULE);
    if (cs_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kCsModuleSection };
      auto cs_modules = config.Get(real_key , kPathsValue);
    
      for (const auto& cs_mod : cs_modules) {
        Path path = cs_mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        cs_language_module->LoadScriptModule({
          .name = name ,
          .paths = { path.string() } ,
        });    
      }
    }

    OE_DEBUG("Loading Lua script modules");
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(LanguageModuleType::LUA_MODULE);
    if (lua_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kLuaModuleSection };
      auto lua_modules = config.Get(real_key , kPathsValue);

      for (const auto& lua_mod : lua_modules) {
        Path path = lua_mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        lua_language_module->LoadScriptModule({
          .name = name ,
          .paths = { path.string() } ,
        });
      }
    }

    OnLoad();
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
      CHECKGL();

      /// go through and trigger any events to dispatch in the next call
      if (Renderer::IsWindowFocused() && lost_window_focus) {
        lost_window_focus = false;

        if (project_metadata->EditorDirectoryChanged()) {
          EventQueue::PushEvent<ProjectDirectoryUpdateEvent>(EDITOR_DIR);
        }

        if (project_metadata->ScriptDirectoryChanged()) {
          EventQueue::PushEvent<ProjectDirectoryUpdateEvent>(SCRIPT_DIR);
        }

        /// update script watchers
        ScriptEngine::UpdateScripts();
      } else {
        lost_window_focus = true;
      }

      /// physics step
      
      /// process any other updates that might trigger events here
      
      /// process all events queued from updating
      EventQueue::Poll(GetEngine() , this);

      /// process any updates that are the result of events here
      
      /// if the window is focuseed we update the application and then the scene
      if (Renderer::IsWindowFocused()) {
        DoUpdate(dt);
      }
        
      Renderer::BeginFrame();
      DoRender();
      Renderer::EndFrame();

      DoRenderUI();
      Renderer::SwapBuffers();

      frame_rate_enforcer.Enforce();
    } 

    Detach();

    OE_DEBUG("Application successfully attached");
  }

  void App::Unload() {
    OE_DEBUG("Unloading application");

    OnUnload();
    
    asset_handler = nullptr;

    layer_stack->Clear(); 
    layer_stack = nullptr;

    if (is_editor) {
      OE_DEBUG(" > Editor unloaded");
      return;
    }
    
    OE_DEBUG("Unloading Lua script modules");
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(LanguageModuleType::LUA_MODULE);
    if (lua_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kLuaModuleSection };
      auto lua_modules = config.Get(real_key , kPathsValue);

      for (const auto& lua_mod : lua_modules) {
        Path path = lua_mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        lua_language_module->UnloadScriptModule(name);
      }
    } else {
      OE_ERROR("Failed to load lua script modules!");
    }
    
    OE_DEBUG("Unloading C# script modules");
    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(LanguageModuleType::CS_MODULE);
    if (cs_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kCsModuleSection };
      auto cs_modules = config.Get(real_key , kPathsValue);
    
      for (const auto& cs_mod : cs_modules) {
        Path path = cs_mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        OE_DEBUG(" > Unloading C# script module {}" , name);
        cs_language_module->UnloadScriptModule(name);
      }
    } else {
      OE_ERROR("Failed to load C# script modules!");
    }

    OE_DEBUG("Shutting down script engine");
    ScriptEngine::Shutdown();

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
    EventHandler handler(event);
    handler.Handle<ProjectDirectoryUpdateEvent>([this](ProjectDirectoryUpdateEvent& e) -> bool {
      if (!is_editor) {
        return true;
      } else if (!project_metadata->RegenProjectFile()) {
        OE_ERROR("Failed to generate project files! Project metadata corrupted!");
        return true;
      } else {
        OE_INFO("Project files reloaded");
      }

      ReloadScripts();

      return true;
    });

    handler.Handle<ScriptReloadEvent>([this](ScriptReloadEvent& e) -> bool {
      if (!is_editor) {
        return false;
      }

      ReloadScripts();
      return true;
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

    Renderer::SetSceneContext(scn_metadata->scene);
    
    /// alert the client app new scene is loaded 
    OnSceneLoad(ActiveScene());
  }
      
  bool App::HasActiveScene() {
    return scene_manager->ActiveScene() != nullptr;
  }

  const SceneMetadata* App::ActiveScene() {
    return scene_manager->ActiveScene();
  }

  void App::UnloadScene() {
    if (ActiveScene() == nullptr) {
      return;
    }
    
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
    for (auto itr = layer_stack->begin(); itr != layer_stack->end(); ++itr) {
      (*itr)->Attach();
    }
    
    OnAttach();
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
