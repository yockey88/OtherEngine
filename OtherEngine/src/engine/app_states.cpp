/**
 * \file engine/app_states.cpp
 **/
#include "engine/app_states.hpp"

#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "engine/editor_states.hpp"
#include "engine/engine.hpp"

#include "application/app_state.hpp"

namespace other {

  Ref<EngineState> AppIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::APP_DETACHED) {
      OE_ASSERT(AppState::IsAttached(), "Application is not attached");
      AppState::DetachApplication();

      if (AppState::exit_code.has_value()) {
        return NewRef<EngineShutdown>(engine);
      } else {
        return NewRef<EngineIdle>(engine);
      }
    }

    if (event == EngineStateEvent::SCENE_LOADED) {
      return NewRef<SceneRunning>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      return NewRef<EngineShutdown>(engine);
    }

    return nullptr;
  }

  void AppIdle::OnAttach() {
    AppState::AttachApplication();
  }

  void AppIdle::OnStep() {
    OE_ASSERT(AppState::HasPrimaryScene(), "Can not load runtime AppIdle state without a default scene");
    AppState::LoadPrimaryScene();
    engine->EngineEvent(EngineStateEvent::SCENE_LOADED);
  }

  void AppIdle::OnDetach() {
  }

  Ref<EngineState> SceneRunning::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::SCENE_UNLOADED) {
      return NewRef<AppIdle>(engine);
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

  void SceneRunning::OnAttach() {
    OE_ASSERT(AppState::Scenes()->HasActiveScene(), "No active scene to attach");
    OE_ASSERT(AppState::Scenes()->ActiveScene() != nullptr, "Cannot attach null scene!");
    OE_ASSERT(AppState::Scenes()->ActiveScene()->scene != nullptr, "Active scene has no scene object");
    AppState::Scenes()->StartScene();
    OE_DEBUG("Scene Idle Attached : {}", AppState::Scenes()->ActiveScene()->name);
  }

  void SceneRunning::OnStep() {
    AppState::RunEarlyUpdate();
    AppState::RunUpdate();
    AppState::RunLateUpdate();
    AppState::HandleRender();
  }

  void SceneRunning::OnDetach() {
    AppState::Scenes()->UnloadActive();
  }

}  // namespace other