/**
 * \file testing_core/test_engine_states.hpp
 **/
#ifndef OTHER_TEST_ENGINE_STATES_HPP
#define OTHER_TEST_ENGINE_STATES_HPP

#include "engine/app_states.hpp"
#include "engine/engine_states.hpp"

namespace other {

  struct TestIdle : public AppIdle {
    TestIdle(Engine* engine)
        : AppIdle(engine, EngineStateTypes::TEST_IDLE) {}
    virtual ~TestIdle() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnStep() override;
  };

  struct TestRunning : public AppIdle {
    TestRunning(Engine* engine)
        : AppIdle(engine, EngineStateTypes::TEST_RUNNING) {}
    virtual ~TestRunning() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

  struct TestComplete : public AppIdle {
    TestComplete(Engine* engine)
        : AppIdle(engine, EngineStateTypes::TEST_COMPLETE) {}
    virtual ~TestComplete() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    /// don't want app to re-attach here
    virtual void OnAttach() override { /* no-op */ }
    virtual void OnStep() override;
  };

}  // namespace other

#endif  // !OTHER_TEST_ENGINE_STATES_HPP