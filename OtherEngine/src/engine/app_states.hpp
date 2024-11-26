/**
 * \file engine/app_states.hpp
 **/
#ifndef OTHER_ENGINE_APP_STATES_HPP
#define OTHER_ENGINE_APP_STATES_HPP

#include "engine/engine_states.hpp"

namespace other {

  class Engine;

  struct AppIdle : public EngineState {
    AppIdle(Engine* engine, EngineStateTypes type = EngineStateTypes::APP_IDLE)
        : EngineState(engine, type) {}
    virtual ~AppIdle() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

  struct SceneRunning : public EngineState {
    SceneRunning(Engine* engine)
        : EngineState(engine, EngineStateTypes::SCENE_IDLE) {}
    virtual ~SceneRunning() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_APP_STATES_HPP