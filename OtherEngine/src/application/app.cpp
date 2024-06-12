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
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "layers/core_layer.hpp"
#include "layers/debug_layer.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  App::App(Engine* engine) 
    :  cmdline(engine->cmd_line) , config(engine->config) , engine_handle(engine) {
  }

  App::~App() {}

  void App::LoadMetadata(const Ref<Project>& metadata) {
    project_metadata = metadata;
  }
      
  Ref<Project> App::GetProjectContext() {
    return Ref<Project>::Clone(project_metadata);
  }

  void App::OnLoad() {
    OE_DEBUG("Loading application");

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
    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(kCsModuleSection);
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
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(kLuaModuleSection);
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

      /// first we check to see if we need to reload any scripts
      
      if (Renderer::IsWindowFocused()) {
        /// this updates the internal representation of the udpate, the actual script 'OnUpdate'
        ///   method is not called until the scene calls it if the scene is currently running
        ScriptEngine::UpdateScripts();
      }

      // updates mouse/keyboard/any connected controllers
      IO::Update();
      EventQueue::Poll(GetEngine() , this);
        
      if (Renderer::IsWindowFocused()) {
        // update all layers
        for (size_t i = 0; i < layer_stack->Size(); ++i) {
          (*layer_stack)[i]->Update(dt);
        }

        DoUpdate(dt);

        UpdateSceneContext(dt);
      }

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
    OE_DEBUG("Unloading application");
    
    asset_handler = nullptr;

    layer_stack->Clear(); 
    layer_stack = nullptr;

    if (is_editor) {
      OE_DEBUG(" > Editor unloaded");
      return;
    }
    
    OE_DEBUG("Unloading Lua script modules");
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(kLuaModuleSection);
    if (lua_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kLuaModuleSection };
      auto lua_modules = config.Get(real_key , kPathsValue);

      for (const auto& lua_mod : lua_modules) {
        Path path = lua_mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        lua_language_module->UnloadScriptModule(name);
      }
    }
    
    OE_DEBUG("Unloading C# script modules");
    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(kCsModuleSection);
    if (cs_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kCsModuleSection };
      auto cs_modules = config.Get(real_key , kPathsValue);
    
      for (const auto& cs_mod : cs_modules) {
        Path path = cs_mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        OE_DEBUG(" > Unloading C# script module {}" , name);
        cs_language_module->UnloadScriptModule(name);
      }
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
    if (event->Type() == EventType::SCRIPT_RELOAD) {
      Opt<Path> active_path = std::nullopt;
      if (scene_manager->ActiveScene() != nullptr) {
        active_path = scene_manager->ActiveScene()->path;
        UnloadScene();
      } 
        
      scene_manager->ClearScenes();
      ScriptEngine::ReloadAllScripts();

      if (active_path.has_value()) {
        LoadScene(active_path.value());
      }
    }

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
    }

    /// propogate scene loading through layers
    for (size_t i = 0; i < layer_stack->Size(); ++i) {
      (*layer_stack)[i]->LoadScene(scene_manager->ActiveScene());
    }
    
    /// alert the client app new scene is loaded 
    OnSceneLoad(scene_manager->ActiveScene());
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

    /// propogate scene unload through layers
    for (size_t i = 0; i < layer_stack->Size(); ++i) {
      (*layer_stack)[i]->LoadScene(nullptr);
    }

    /// alert client app about scene unload
    OnSceneUnload();
  }

  void App::Attach() {
    OE_DEBUG("Attaching application");
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
    OE_DEBUG("Detaching application");

    UnloadScene();

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

  void App::SetInEditor() {
    in_editor = true;
  }

} // namespace other
