/**
 * \file application/app_state.cpp
 **/
#include "application/app_state.hpp"

#include <imgui/imgui.h>

#include "core/config_keys.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "application/app.hpp"
#include "event/event_queue.hpp"
#include "event/scene_events.hpp"
#include "input/io.hpp"

#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  Ref<StateMachine> AppState::state = nullptr;
  Ref<AppState::Data> AppState::data = nullptr;

  void AppState::Initialize(const CmdLine& cmd_line, const ConfigTable& config, App* app_handle) {
    OE_ASSERT(app_handle != nullptr, "Can not load null application");
    Filesystem::Initialize(cmd_line, config);
    data = NewRef<Data>(app_handle, Ref<Project>::Create(cmd_line, config));
    state = NewRef<AppStateMachine>();
    data->cmd_line = cmd_line;
    data->config = config;

    data->app_handle->Load();
  }

  void AppState::Shutdown() {
    for (auto& [id, window] : data->ui_windows) {
      window->Detach();
    }
    data->ui_windows.clear();

    data->layers->InvokeControlledLoop(&Layer::Detach);
    data->layers->Clear();

    data->app_handle->Unload();
    data = nullptr;
  }

  CmdLine& AppState::GetProcessArguments() {
    OE_ASSERT(data != nullptr, "Can not access apps data until app is loaded");
    return data->cmd_line;
  }

  ConfigTable& AppState::GetLoadedConfig() {
    OE_ASSERT(data != nullptr, "Can not access apps data until app is loaded");
    return data->config;
  }

  Ref<Project> AppState::ProjectContext() {
    OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
    return Ref<Project>::Clone(data->project);
  }

  Ref<AssetHandler> AppState::Assets() {
    OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
    return Ref<AssetHandler>::Clone(data->assets);
  }

  Scope<LayerStack>& AppState::Layers() {
    OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
    return data->layers;
  }

  Scope<SceneManager>& AppState::Scenes() {
    OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
    return data->scenes;
  }

  UUID AppState::PushUIWindow(Ref<UIWindow> window) {
    OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
    UUID id = FNV(window->Title());
    data->ui_windows.AddWindow(id, window);
    return id;
  }

  void AppState::PopUIWindow(UUID id) {
    OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
    data->ui_windows.RemoveWindow(id);
    return;
  }

  UUID AppState::PushLayer(Ref<Layer> layer) {
    OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
    OE_ASSERT(layer != nullptr, "Can not push null layer into layer stack");
    data->layers->PushLayer(layer);
    return layer->GetUUID();
    ;
  }

  void AppState::PopLayer(Opt<UUID> id) {
    OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
    if (data->layers->Empty()) {
      OE_WARN("Attempting to pop a layer from an empty layer stack");
      return;
    }

    if (id.has_value()) {
      data->layers->PopLayer(id.value());
    } else {
      data->layers->PopLayer();
    }
  }

  App& AppState::AppHandle() {
    return *data->app_handle;
  }

  void AppState::AppEvent(const Ref<AppStateEvent>& event) {
    state->DispatchEvent(event);
  }

  Ref<AppState::Data> AppState::GetData() {
    OE_ASSERT(data != nullptr, "can not access application state until it is loaded");
    return data;
  }

  void AppState::AttachApplication() {
    auto& proj_meta = data->project->GetMetadata();

    EventQueue::RegisterEventDispatcher<SceneLoad>(
      "App-State-Scene-Load-Handler",
      { &AppState::HandleSceneLoad }
    );

    ScriptEngine::LoadProjectModules();

    ScriptEngine::LoadAttachments("scene");
    ScriptEngine::LoadAttachments("ui");

    data->app_handle->Attach();

    data->assets = data->app_handle->CreateAssetHandler();
    data->scenes->LoadRenderer(data->app_handle->CreateSceneRenderer());

    bool need_primary = data->config.GetVal<bool>(kProjectSection, kNeedPrimarySceneValue, false).value_or(true);
    if (need_primary && !proj_meta.primary_scene.has_value()) {
      OE_WARN("Primary Scene marked as present but no primary scene proved (config is corrupt)");
    } else if (need_primary && proj_meta.primary_scene.has_value()) {
      auto primary_scene = FindSceneFileByName(*proj_meta.primary_scene);

      if (!primary_scene.has_value()) {
        OE_WARN("Could not find primary scene : {}", *proj_meta.primary_scene);
      } else if (data->scenes->LoadScene(*primary_scene)) {
        data->scenes->SetAsActive(*primary_scene);
      }
    }
  }

  void AppState::DetachApplication() {
    data->layers->InvokeControlledLoop(&Layer::Detach);
    data->layers->Clear();

    data->scenes->UnloadActive();
    data->app_handle->Detach();

    ScriptEngine::UnloadAttachments();
    ScriptEngine::UnloadProjectModules();
  }

  void AppState::OnEngineTick(float dt) {
    data->frame_delta = dt;
    Filesystem::Poll();
    IO::Update();

    // AppState::RunEarlyUpdate();
    // AppState::RunUpdate();
    // AppState::RunLateUpdate();
    // AppState::HandleRender();

    state->Step();
  }

  void AppState::RunEarlyUpdate() {
    if (!Renderer::IsWindowFocused()) {
      return;
    }
    data->app_handle->DoEarlyUpdate(data->frame_delta);
    data->layers->InvokeControlledLoop(&Layer::EarlyUpdate, data->frame_delta);
    data->scenes->EarlyUpdateScene(data->frame_delta);
  }

  void AppState::RunUpdate() {
    if (!Renderer::IsWindowFocused()) {
      return;
    }

    data->app_handle->DoUpdate(data->frame_delta);
    data->layers->InvokeControlledLoop(&Layer::Update, data->frame_delta);
    data->scenes->UpdateScene(data->frame_delta);
  }

  void AppState::RunLateUpdate() {
    if (!Renderer::IsWindowFocused()) {
      return;
    }

    data->app_handle->DoLateUpdate(data->frame_delta);
    data->layers->InvokeControlledLoop(&Layer::LateUpdate, data->frame_delta);
    data->scenes->LateUpdateScene(data->frame_delta);

    /// run update on state machine
    // state->Step(data->frame_delta);

    /// update ui windows after all other updates
    for (auto& [id, window] : data->ui_windows) {
      window->Update(data->frame_delta);
    }
  }

  void AppState::HandleRender() {
    Renderer::GetWindow()->Clear();

    data->app_handle->DoRender();
    data->layers->InvokeControlledLoop(&Layer::Render);
    data->scenes->RenderScene();

    if (UI::Enabled()) {
      UI::BeginFrame();
      data->layers->InvokeControlledLoop(&Layer::UIRender);
      data->app_handle->DoRenderUI();

      for (auto& [id, window] : data->ui_windows) {
        window->Render();
      }

      // ScriptEngine::RenderUI();

      // /// lambda to get fps from delta
      // auto fps = [](float dt) -> float {
      //   return (1.f / dt) * 1000.f;
      // };

      // sandbox_ui->RenderUI();
      // const ImVec2 win_size = { (float)Renderer::WindowSize().x, (float)Renderer::WindowSize().y };

      // if (ImGui::Begin("Frames")) {
      //   if (!success) {
      //     ScopedColor red(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
      //     ImGui::Text("Failed to render frame");
      //   } else {
      //     ImGui::Text("Frames %llu", frames.size());
      //     if (auto frame = frames.find(FNV("Debug")); frame != frames.end()) {
      //       RenderItem(frame->second->texture, "Debug", ImVec2(win_size.x, win_size.y));
      //     }
      //   }
      // }
      // ImGui::End();
      UI::EndFrame();
    }
    Renderer::GetWindow()->SwapBuffers();
  }

  Opt<Path> AppState::FindSceneFileByName(const std::string_view name) {
    auto scene_dir = data->project->GetMetadata().assets_dir / "scenes";
    for (auto& entry : std::filesystem::directory_iterator(scene_dir)) {
      if (entry.is_regular_file() && entry.path().stem() == name) {
        return entry.path();
      }
    }

    return std::nullopt;
  }

  bool AppState::HandleSceneLoad(SceneLoad& event) {
    state->DispatchEvent(NewRef<SceneLoaded>(event.scene_id));
    return false;
  }

  AppState::Data::Data(App* app_handle, Ref<Project> proj)
      : app_handle(app_handle) {
    OE_ASSERT(app_handle != nullptr, "Can not load null application");
    /// TODO:
    ///   add override ability
    ///  state = app_handle->HookStateControl();
    state = NewRef<AppStateMachine>();

    layers = NewScope<LayerStack>();
    scenes = NewScope<SceneManager>();
    project = Ref<Project>::Clone(proj);
  }

  AppState::Data::~Data() {
    state = nullptr;

    delete app_handle;
    app_handle = nullptr;
    layers = nullptr;
    scenes = nullptr;
    app_handle = nullptr;
    assets = nullptr;
    project = nullptr;
  }

}  // namespace other
