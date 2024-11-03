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

#include "engine/engine.hpp"

namespace other {

  // Ref<StateMachine> AppState::state = nullptr;
  Ref<AppState::Data> AppState::data = nullptr;

  void AppState::Initialize(const CmdLine& cmd_line, const ConfigTable& config) {
    // OE_ASSERT(app_handle != nullptr, "Can not load null application");

    data = NewRef<Data>(NewApp(cmd_line, config), Ref<Project>::Create(cmd_line, config));
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

  bool AppState::HasAppLoaded() {
    return data != nullptr;
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

  Ref<SceneRenderer> AppState::GetSceneRenderer() {
    OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
    return data->scenes->GetRenderer();
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

  void AppState::AppEvent(const Ref<EngineStateEvent>& event) {
    // state->DispatchEvent(event);
  }

  Ref<AppState::Data> AppState::GetData() {
    OE_ASSERT(data != nullptr, "can not access application state until it is loaded");
    return data;
  }

  void AppState::AttachApplication() {
    ScriptEngine::LoadProjectModules();
    ScriptEngine::LoadAttachments("scene");
    ScriptEngine::LoadAttachments("ui");

    data->app_handle->Attach();
    /// initial flush of events, poll filesystem in case attaching created new files
    ///   and poll event queue to ensure all update events are processed before stepping
    Filesystem::Poll();
    EventQueue::Poll();

    // then create application objects
    data->assets = data->app_handle->CreateAssetHandler();
    data->scenes->LoadRenderer(data->app_handle->CreateSceneRenderer());
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
    if (data != nullptr) {
      data->frame_delta = dt;
    }
    Filesystem::Poll();
    IO::Update();
    EventQueue::Poll();
  }

  void AppState::RunEarlyUpdate() {
    data->app_handle->DoEarlyUpdate(data->frame_delta);
    data->layers->InvokeControlledLoop(&Layer::EarlyUpdate, data->frame_delta);
    data->scenes->EarlyUpdateScene(data->frame_delta);
  }

  void AppState::RunUpdate() {
    data->app_handle->DoUpdate(data->frame_delta);
    data->layers->InvokeControlledLoop(&Layer::Update, data->frame_delta);
    data->scenes->UpdateScene(data->frame_delta);
  }

  void AppState::RunLateUpdate() {
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

    data->scenes->GetRenderer()->ClearPipelines();
    data->app_handle->OnRender();
    data->layers->InvokeControlledLoop(&Layer::Render);
    if (!data->scenes->RenderScene()) {
      OE_ERROR("Failed to render scene");
    };

    if (UI::Enabled()) {
      UI::BeginFrame();

      data->app_handle->OnRenderUI();
      data->layers->InvokeControlledLoop(&Layer::UIRender);
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

  AppState::Data::Data(App* app_handle, Ref<Project> proj)
      : app_handle(app_handle) {
    OE_ASSERT(app_handle != nullptr, "Can not load null application");

    layers = NewScope<LayerStack>();
    scenes = NewScope<SceneManager>();
    project = Ref<Project>::Clone(proj);
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
