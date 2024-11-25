/**
 * \file testing_core/test_engine_states.cpp
 **/
#include "testing_core/test_engine_states.hpp"

#include "engine/engine.hpp"

#include "application/app_state.hpp"

#include "mock_engine.hpp"

namespace other {

  Ref<EngineState> TestIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::TEST_START) {
      return NewRef<TestRunning>(engine);
    }

    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      return NewRef<EngineShutdown>(engine);
    }

    return nullptr;
  }

  void TestIdle::OnStep() {
    MockEngine* mock_engine = MockEngine::TestEngine();
    mock_engine->InitializeTest();
  }

  Ref<EngineState> TestRunning::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::TEST_END) {
      OE_ASSERT(AppState::IsAttached(), "Application is not attached");
      AppState::DetachApplication();
      return NewRef<TestComplete>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      engine->EngineEvent(EngineStateEvent::APP_DETACHED);
      return NewRef<AppIdle>(engine);
    }

    return nullptr;
  }

  void TestRunning::OnAttach() {
    /// start test
  }

  void TestRunning::OnStep() {
    MockEngine::TestEngine()->UpdateTest();
    MockEngine::TestEngine()->RenderTest();
  }

  void TestRunning::OnDetach() {
    /// finalize test
  }

  Ref<EngineState> TestComplete::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      return NewRef<EngineShutdown>(engine);
    }

    return nullptr;
  }

  void TestComplete::OnStep() {
    /// finalize test
  }

}  // namespace other