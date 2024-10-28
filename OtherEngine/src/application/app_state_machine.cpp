/**
 * \file application/app_state_machine.cpp
 **/
#include "application/app_state_machine.hpp"

#include "application/app_state.hpp"

namespace other {

  Ref<ApplicationState> EngineIdle::HandleEvent(const Ref<AppStateEvent>& event) {
    if (event->type == AppStateEvents::APP_ATTACHED) {
      AppState::AttachApplication();
      return NewRef<AppIdle>();
    }

    return nullptr;
  }

  Ref<ApplicationState> AppIdle::HandleEvent(const Ref<AppStateEvent>& event) {
    if (event->type == AppStateEvents::APP_DETACHED) {
      AppState::DetachApplication();
      return NewRef<EngineIdle>();
    }

    if (event->type == AppStateEvents::SCENE_LOADED) {
      return NewRef<SceneIdle>();
    }

    return nullptr;
  }

  void AppIdle::OnStep() {}

  Ref<ApplicationState> SceneIdle::HandleEvent(const Ref<AppStateEvent>& event) {
    if (event->type == AppStateEvents::SCENE_UNLOADED) {
      return NewRef<AppIdle>();
    }

    return nullptr;
  }

  void SceneIdle::OnStep() {
    AppState::RunEarlyUpdate();
    AppState::RunUpdate();
    AppState::RunLateUpdate();
    AppState::HandleRender();
  }

}  // namespace other