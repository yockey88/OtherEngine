/**
 * \file editor/editor_states.cpp
 **/
#include "editor/editor_states.hpp"

#include "core/filesystem.hpp"
#include "engine/engine.hpp"
#include "engine/engine_states.hpp"
#include "environment/environment.hpp"

#include "application/app_state.hpp"
#include "event/event_queue.hpp"
#include "event/scene_events.hpp"

#include "rendering/renderer.hpp"

#include "editor/editor_layer.hpp"

namespace other {

  Ref<EngineState> EditorIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      AppState::PopLayer(editor_layer_id);
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN || event == EngineStateEvent::APP_DETACHED) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      AppState::DetachApplication();
      /// editor layer popped on shutdown
      return NewRef<EngineShutdown>(engine);
    }

    if (event == EngineStateEvent::EDIT_SCENE) {
      return NewRef<EditingScene>(engine);
    }

    return nullptr;
  }

  void EditorIdle::OnAttach() {
    OE_DEBUG("Editor Idle Attached");
  }

  void EditorIdle::OnStep() {
    PROFILE_SECTION("EditorIdle--OnStep");
    if (AppState::IsLoading()) {
      Ref<EditorLayer> layer = AppState::PushLayer<EditorLayer>();
      OE_ASSERT(layer != nullptr, "Failed to push editor layer");
      editor_layer_id = layer->GetUUID();

      AppState::MarkLoaded();

      Environment::Get().terminal_open = true;
      if (AppState::HasPrimaryScene()) {
        AppState::LoadPrimaryScene();
        return;
      }
    }

    Filesystem::Poll();
    AppState::FlushUpdateLoop();

    if (!AppState::exit_code.has_value()) {
      AppState::HandleRender();
    }
  }

  void EditorIdle::OnDetach() {
    EventQueue::UnregisterEventDispatcher("EditorIdle--SceneActive");
  }

  Ref<EngineState> EditingScene::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::SCENE_UNLOADED) {
      return NewRef<EditorIdle>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      /// this returns app idle so the scene gets unloaded and we queue
      ///   app detached event so that app-idle state gets immediately detached
      ///   and the engine can shutdown
      engine->EngineEvent(EngineStateEvent::APP_DETACHED);
      return NewRef<EditorIdle>(engine);
    }

    return nullptr;
  }

  void EditingScene::OnAttach() {
    OE_ASSERT(AppState::Scenes()->HasActiveScene(), "No active scene to edit");
    OE_ASSERT(AppState::Scenes()->ActiveScene() != nullptr, "Cannot edit null scene!");
    OE_ASSERT(AppState::Scenes()->ActiveScene()->scene != nullptr, "Active scene has no scene object");

    OE_DEBUG("Scene Editor Attached : {}", AppState::Scenes()->ActiveScene()->name);
  }

  void EditingScene::OnStep() {
    PROFILE_SECTION("EditingScene--OnStep");

    bool should_poll = Renderer::IsWindowFocused();
    if (should_poll) {
      Filesystem::Poll();
    }

    AppState::FlushUpdateLoop();
    AppState::HandleRender();
  }

  void EditingScene::OnDetach() {
    AppState::Scenes()->UnloadActive();
  }

  bool EditingScene::HandleSceneUnload(SceneUnload& event) {
    engine->EngineEvent(EngineStateEvent::SCENE_UNLOADED);
    return false;
  }

}  // namespace other