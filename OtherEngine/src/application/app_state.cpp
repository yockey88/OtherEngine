/**
 * \file application/app_state.cpp
 **/
#include "application/app_state.hpp"

#include "core/config_keys.hpp"
#include "core/logger.hpp"

#include "application/app.hpp"
#include "event/event_queue.hpp"

#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  Scope<AppState::Data> AppState::state = nullptr;

  void AppState::Initialize(const CmdLine& cmd_line, const ConfigTable& config, App* app_handle) {
    OE_ASSERT(app_handle != nullptr, "Can not load null application");
    state = NewScope<Data>(app_handle, Ref<Project>::Create(cmd_line, config));
    state->cmd_line = cmd_line;
    state->config = config;

    state->app_handle->Load();
  }

  void AppState::Shutdown() {
    for (auto& [id, window] : state->ui_windows) {
      window->OnDetach();
    }
    state->ui_windows.clear();

    state->layers->InvokeControlledLoop(&Layer::Detach);
    state->layers->Clear();

    state->app_handle->Unload();
    state = nullptr;
  }

  Ref<Project> AppState::ProjectContext() {
    OE_ASSERT(state != nullptr, "Can not access app state until app is loaded");
    return Ref<Project>::Clone(state->project);
  }

  Ref<AssetHandler> AppState::Assets() {
    OE_ASSERT(state != nullptr, "Can not access app state until app is loaded");
    return Ref<AssetHandler>::Clone(state->assets);
  }

  Scope<LayerStack>& AppState::Layers() {
    OE_ASSERT(state != nullptr, "Can not access app state until app is loaded");
    return state->layers;
  }

  Scope<SceneManager>& AppState::Scenes() {
    OE_ASSERT(state != nullptr, "Can not access app state until app is loaded");
    return state->scenes;
  }

  UUID AppState::PushUIWindow(Ref<UIWindow> window) {
    OE_ASSERT(state != nullptr, "Can not access app state until app is loaded");
    UUID id = FNV(window->Title());
    state->ui_windows.AddWindow(id, window);
    return id;
  }

  void AppState::PopUIWindow(UUID id) {
    OE_ASSERT(state != nullptr, "Can not access app state until app is loaded");
    state->ui_windows.RemoveWindow(id);
    return;
  }

  UUID AppState::PushLayer(Ref<Layer> layer) {
    OE_ASSERT(state != nullptr, "Can not access app state until app is loaded");
    OE_ASSERT(layer != nullptr, "Can not push null layer into layer stack");
    state->layers->PushLayer(layer);
    return layer->GetUUID();
    ;
  }

  void AppState::PopLayer(Opt<UUID> id) {
    OE_ASSERT(state != nullptr, "Can not access app state until app is loaded");
    if (state->layers->Empty()) {
      OE_WARN("Attempting to pop a layer from an empty layer stack");
      return;
    }

    if (id.has_value()) {
      state->layers->PopLayer(id.value());
    } else {
      state->layers->PopLayer();
    }
  }

  App& AppState::AppHandle() {
    return *state->app_handle;
  }

  void AppState::OnEngineTick(float dt) {
    state->frame_delta = dt;
  }

  Opt<Path> AppState::FindSceneFileByName(const std::string_view name) {
    auto scene_dir = state->project->GetMetadata().assets_dir / "scenes";
    for (auto& entry : std::filesystem::directory_iterator(scene_dir)) {
      if (entry.is_regular_file() && entry.path().stem() == name) {
        return entry.path();
      }
    }

    return std::nullopt;
  }

  void AppState::AttachApplication() {
    state->app_handle->Attach();
    // state->project->LoadFiles();
    ScriptEngine::LoadProjectModules();

    auto& proj_meta = state->project->GetMetadata();
    bool need_primary = state->config.GetVal<bool>(kProjectSection, kNeedPrimarySceneValue, false).value_or(true);
    if (need_primary && !proj_meta.primary_scene.has_value()) {
      OE_WARN("Primary Scene marked as present but no primary scene proved (config is corrupt)");
    } else if (need_primary && proj_meta.primary_scene.has_value()) {
      auto primary_scene = FindSceneFileByName(*proj_meta.primary_scene);

      if (!primary_scene.has_value()) {
        OE_WARN("Could not find primary scene : {}", *proj_meta.primary_scene);
      } else if (state->scenes->LoadScene(*primary_scene)) {
        state->scenes->SetAsActive(*primary_scene);
      }
    }
  }

  void AppState::DetachApplication() {
    state->scenes->UnloadActive();
    state->layers->InvokeControlledLoop(&Layer::Detach);
    state->layers->Clear();
    state->app_handle->Detach();
    ScriptEngine::UnloadProjectModules();
  }

  void AppState::RunEarlyUpdate() {
    if (!Renderer::IsWindowFocused()) {
      return;
    }
    state->app_handle->DoEarlyUpdate(state->frame_delta);
    state->layers->InvokeControlledLoop(&Layer::EarlyUpdate, state->frame_delta);
    state->scenes->EarlyUpdateScene(state->frame_delta);
  }

  void AppState::RunUpdate() {
    if (!Renderer::IsWindowFocused()) {
      return;
    }

    state->app_handle->DoUpdate(state->frame_delta);
    state->layers->InvokeControlledLoop(&Layer::Update, state->frame_delta);
    state->scenes->UpdateScene(state->frame_delta);
  }

  void AppState::RunLateUpdate() {
    if (!Renderer::IsWindowFocused()) {
      return;
    }

    state->app_handle->DoLateUpdate(state->frame_delta);
    state->layers->InvokeControlledLoop(&Layer::LateUpdate, state->frame_delta);
    state->scenes->LateUpdateScene(state->frame_delta);
  }

  void AppState::HandleRender() {
    Renderer::GetWindow()->Clear();

    state->app_handle->DoRender();
    state->layers->InvokeControlledLoop(&Layer::Render);

    for (auto& [id, window] : state->ui_windows) {
      window->OnUpdate(state->frame_delta);
    }

    if (UI::Enabled()) {
      UI::BeginFrame();
      state->layers->InvokeControlledLoop(&Layer::UIRender);
      state->app_handle->DoRenderUI();

      for (auto& [id, window] : state->ui_windows) {
        window->Render();
      }
      UI::EndFrame();
    }
    Renderer::GetWindow()->SwapBuffers();
  }

  AppState::Data::Data(App* app_handle, Ref<Project> proj)
      : app_handle(app_handle) {
    OE_ASSERT(app_handle != nullptr, "Can not load null application");

    layers = NewScope<LayerStack>();
    scenes = NewScope<SceneManager>();

    project = Ref<Project>::Clone(proj);
    assets = app_handle->CreateAssetHandler();
  }

  AppState::Data::~Data() {
    delete app_handle;
    app_handle = nullptr;
    layers = nullptr;
    scenes = nullptr;
    app_handle = nullptr;
    assets = nullptr;
    project = nullptr;
  }

}  // namespace other
