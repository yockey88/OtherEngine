/**
 * \file application/app_state_machine.hpp
 **/
#ifndef OTHER_ENGINE_APP_STATE_MACHINE_HPP
#define OTHER_ENGINE_APP_STATE_MACHINE_HPP

#include <magic_enum/magic_enum.hpp>

#include "core/state.hpp"

#include "engine/app_states.hpp"
#include "engine/engine_states.hpp"


namespace other {

  class EngineStateMachine : public StateMachine<EngineState, EngineStateEvent> {
   public:
    EngineStateMachine(Engine* engine)
        : StateMachine<EngineState, EngineStateEvent>(NewRef<EngineLaunching>(engine)), engine(engine) {}
    virtual ~EngineStateMachine() override {}

    Engine* engine = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_APP_STATE_MACHINE_HPP