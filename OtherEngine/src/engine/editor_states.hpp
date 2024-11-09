/**
 * \file engine/editor_states.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_STATES_HPP
#define OTHER_ENGINE_EDITOR_STATES_HPP

#include "engine/engine_states.hpp"

namespace other {

  class Engine;

  struct EditorIdle : public EngineState {
    EditorIdle(Engine* engine)
        : EngineState(engine, EngineStateTypes::EDITOR_IDLE) {}
    virtual ~EditorIdle() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

  struct EditingScene : public EngineState {
    EditingScene(Engine* engine)
        : EngineState(engine, EngineStateTypes::EDITING_SCENE) {}
    virtual ~EditingScene() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EDITOR_STATES_HPP