/**
 * \file application/app_state_machine.hpp
 **/
#ifndef OTHER_ENGINE_APP_STATE_MACHINE_HPP
#define OTHER_ENGINE_APP_STATE_MACHINE_HPP

#include "core/state.hpp"
#include "core/uuid.hpp"

namespace other {

  enum class AppStateEvents : uint64_t {
    NO_EVENT = 0,

    APP_ATTACHED = bit(1),
    APP_DETACHED = bit(2),

    SCENE_LOADED = bit(3),
    SCENE_UNLOADED = bit(4),
  };

  enum class AppStateTypes : uint64_t {
    IDLE = 0,
    ENGINE_IDLE = bit(1),
    APP_IDLE = bit(2),
    SCENE_IDLE = bit(3),
  };

  struct AppStateEvent : public StateEvent {
    AppStateEvent(AppStateEvents type)
        : type(type) {}
    virtual ~AppStateEvent() override {}

    AppStateEvents type;

    // Ref<AppState::Data> data = nullptr;
  };

  struct ApplicationAttached : public AppStateEvent {
    ApplicationAttached()
        : AppStateEvent(AppStateEvents::APP_ATTACHED) {}
    virtual ~ApplicationAttached() override {}
  };

  struct ApplicationDetached : public AppStateEvent {
    ApplicationDetached()
        : AppStateEvent(AppStateEvents::APP_DETACHED) {}
    virtual ~ApplicationDetached() override {}
  };

  struct SceneLoaded : public AppStateEvent {
    SceneLoaded(UUID handle)
        : AppStateEvent(AppStateEvents::SCENE_LOADED), scene_handle(handle) {}
    virtual ~SceneLoaded() override {}

    UUID scene_handle;
  };

  struct ApplicationState : public StateImpl<ApplicationState, AppStateEvent> {
    ApplicationState(AppStateTypes type)
        : type(type) {}
    virtual ~ApplicationState() override {}
    AppStateTypes type;
  };

  struct EngineIdle : public ApplicationState {
    EngineIdle()
        : ApplicationState(AppStateTypes::ENGINE_IDLE) {}
    virtual ~EngineIdle() override {}
    virtual Ref<ApplicationState> HandleEvent(const Ref<AppStateEvent>& event) override;
  };

  struct AppIdle : public ApplicationState {
    AppIdle()
        : ApplicationState(AppStateTypes::APP_IDLE) {}
    virtual ~AppIdle() override {}
    virtual Ref<ApplicationState> HandleEvent(const Ref<AppStateEvent>& event) override;

    virtual void OnStep() override;
  };

  struct SceneIdle : public ApplicationState {
    SceneIdle()
        : ApplicationState(AppStateTypes::SCENE_IDLE) {}
    virtual ~SceneIdle() override {}
    virtual Ref<ApplicationState> HandleEvent(const Ref<AppStateEvent>& event) override;

    virtual void OnStep() override;
  };

  class AppStateMachine : public StateMachineImpl<ApplicationState, AppStateEvent> {
   public:
    AppStateMachine()
        : StateMachineImpl<ApplicationState, AppStateEvent>(NewRef<EngineIdle>()) {}
    virtual ~AppStateMachine() override {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_APP_STATE_MACHINE_HPP