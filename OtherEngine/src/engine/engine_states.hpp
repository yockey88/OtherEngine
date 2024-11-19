/**
 * \file engine/engine_states.hpp
 **/
#ifndef OTHER_ENGINE_ENGINE_STATES_HPP
#define OTHER_ENGINE_ENGINE_STATES_HPP

#include "core/logger.hpp"
#include "core/state.hpp"

namespace other {

  enum class EngineStateEvent : uint64_t {
    NO_EVENT = 0,

    EDITOR_START,
    EDIT_SCENE,

    ENGINE_LOAD_FINISHED,
    ENGINE_UNLOAD_FINISHED,
    ENGINE_SHUTDOWN,

    APP_ATTACHED,
    APP_DETACHED,

    SCENE_LOADED,
    SCENE_UNLOADED,

    CORRUPT_CONFIG_ERROR,
    CORRUPT_SHADER_ERROR,
    ENGINE_FAILURE,

    NUM_ENGINE_EVENTS,
    INVALID_ENGINE_EVENT = NUM_ENGINE_EVENTS,
  };

  enum class EngineStateTypes : uint64_t {
    IDLE = 0,

    ENGINE_LAUNCHING,
    ENGINE_IDLE,
    ENGINE_SHUTDOWN,

    EDITOR_IDLE,
    EDITING_SCENE,

    APP_IDLE,
    SCENE_IDLE,

    ERROR_STATE,

    NUM_ENGINE_STATES,
    INVALID_ENGINE_STATE = NUM_ENGINE_STATES,
  };

  class Engine;

  struct EngineState : public StateImpl<EngineState, EngineStateEvent> {
    EngineState(Engine* engine, EngineStateTypes type)
        : type(type), engine(engine) {
      OE_ASSERT(engine != nullptr, "Invalid engine pointer");
    }
    virtual ~EngineState() override {}

    EngineStateTypes type;
    Engine* engine = nullptr;
  };

  struct IdleState : public EngineState {
    IdleState(Engine* engine)
        : EngineState(engine, EngineStateTypes::IDLE) {}
    virtual ~IdleState() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;
  };

  struct ErrorState : public EngineState {
    ErrorState(Engine* engine)
        : EngineState(engine, EngineStateTypes::ERROR_STATE) {}
    virtual ~ErrorState() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    // virtual void OnAttach() override;
    // virtual void OnStep() override;
    // virtual void OnDetach() override;
  };

  struct EngineLaunching : public EngineState {
    EngineLaunching(Engine* engine)
        : EngineState(engine, EngineStateTypes::ENGINE_LAUNCHING) {}
    virtual ~EngineLaunching() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

  struct EngineIdle : public EngineState {
    EngineIdle(Engine* engine)
        : EngineState(engine, EngineStateTypes::ENGINE_IDLE) {}
    virtual ~EngineIdle() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

  struct EngineShutdown : public EngineState {
    EngineShutdown(Engine* engine)
        : EngineState(engine, EngineStateTypes::ENGINE_SHUTDOWN) {}
    virtual ~EngineShutdown() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ENGINE_STATES_HPP