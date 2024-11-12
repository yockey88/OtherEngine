/**
 * \file engine/editor_states.cpp
 **/
#include "engine/editor_states.hpp"

#include "core/config_keys.hpp"
#include "engine/engine.hpp"

#include "application/app_state.hpp"

#include "rendering/renderer.hpp"

#include "editor/editor_layer.hpp"
#include "editor/test_editor_layer.hpp"
#include "engine_states.hpp"

namespace other {

  Ref<EngineState> EditorIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      return NewRef<EngineShutdown>(engine);
    }

    if (event == EngineStateEvent::EDIT_SCENE || event == EngineStateEvent::SCENE_LOADED) {
      return NewRef<EditingScene>(engine);
    }

    return nullptr;
  }

  void EditorIdle::OnAttach() {
    OE_DEBUG("Editor Idle Attached");
    AppState::AttachApplication();

    auto& config = AppState::GetLoadedConfig();
    bool use_test_editor = config.GetVal<bool>(kEditorSection, kTestEditorValue).value_or(false);

    if (use_test_editor) {
      AppState::PushLayer<TEditorLayer>();
    } else {
      AppState::PushLayer<EditorLayer>();
    }
  }

  void EditorIdle::OnStep() {
    if (AppState::HasPrimaryScene()) {
      AppState::LoadPrimaryScene();
      engine->EngineEvent(EngineStateEvent::EDIT_SCENE);
    } else {
      /// load sandbox
      /// return
    }
  }

  void EditorIdle::OnDetach() {
  }

  Ref<EngineState> EditingScene::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      /// this returns app idle so the scene gets unloaded and we queue
      ///   app detached event so that app-idle state gets immediately detached
      ///   and the engine can shutdown
      engine->EngineEvent(EngineStateEvent::APP_DETACHED);
      return NewRef<AppIdle>(engine);
    }

    return nullptr;
  }

  void EditingScene::OnAttach() {
    OE_ASSERT(AppState::Scenes()->HasActiveScene(), "No active scene to edit");
    OE_ASSERT(AppState::Scenes()->ActiveScene() != nullptr, "Cannot edit null scene!");
    OE_ASSERT(AppState::Scenes()->ActiveScene()->scene != nullptr, "Active scene has no scene object");
    // AppState::Scenes()->StartScene();
    OE_DEBUG("Scene Editor Attached : {}", AppState::Scenes()->ActiveScene()->name);

    /// flush the whole loop forcing idle updates
    AppState::RunEarlyUpdate();
    AppState::RunUpdate();
    AppState::RunLateUpdate();
    AppState::HandleRender();
  }

  void EditingScene::OnStep() {
    AppState::RunEarlyUpdate();
    AppState::RunUpdate();
    AppState::RunLateUpdate();
    AppState::HandleRender();
  }

  void EditingScene::OnDetach() {
    AppState::Scenes()->UnloadActive();
  }

}  // namespace other