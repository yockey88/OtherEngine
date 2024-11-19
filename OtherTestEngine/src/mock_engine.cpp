/**
 * \file mock_engine.cpp
 **/
#include "mock_engine.hpp"

#include <stdexcept>

#include <SDL.h>
#include <gtest.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <spdlog/common.h>
#include <spdlog/sinks/callback_sink.h>

#include "testing_core/errors.hpp"

#include "core/logger.hpp"

#include "application/app_state.hpp"
#include "event/event_queue.hpp"

namespace other {

  MockEngine* MockEngine::instance = nullptr;

  MockEngine::MockEngine(CmdLine cmd_line, std::string main_thread_name)
      : Engine(cmd_line, main_thread_name) {
    instance = this;
  }

  MockEngine::~MockEngine() {
    instance = nullptr;
  }

  MockEngine* MockEngine::TestEngine() {
    if (instance == nullptr) {
      throw std::logic_error("Cannot call MockEngine::TestEngine until after test has been loaded!");
    }
    return instance;
  }

  void MockEngine::RegisterMockApplication(const TestDescription& description) {
    run_record.test_name = description.test_name;
  }

  void MockEngine::RecordError(ErrorLevel level, std::source_location loc) {
    run_record.num_errors++;
    records->Record(TestEngineRecords::ERROR_DESCRIPTION, OtherTestEngineError(level, "Error encountered", loc));
  }

  void MockEngine::RecordWarning(std::source_location loc) {
    run_record.num_warnings++;
    records->Record(TestEngineRecords::ERROR_DESCRIPTION, OtherTestEngineError(ErrorLevel::WARNING, "Warning encountered", loc));
  }

  void MockEngine::Run() {
    Start();

    records = NewScope<TestEngineRecords>();

    OE_INFO("Running");
    do {
      try {
        Step();
      } catch (OtherTestEngineError& e) {
        e.num = run_record.num_errors++;

        records->Record(TestEngineRecords::ERROR_DESCRIPTION, e);
        state->HandleEvent(EngineStateEvent::ENGINE_FAILURE);
      } catch (...) {
        OtherTestEngineError err(ErrorLevel::FATAL, "Unknown exception caught at top level");
        err.num = run_record.num_errors++;

        records->Record(TestEngineRecords::ERROR_DESCRIPTION, err);
        state->HandleEvent(EngineStateEvent::ENGINE_FAILURE);
      }
    } while (!exit_code.has_value());

    records = nullptr;

    OE_ASSERT(exit_code.has_value(), "Driver did not set exit code");
    Stop();
  }

  void MockEngine::Start() {
    instance = this;

    MockApp* app = dynamic_cast<MockApp*>(&AppState::AppHandle());
    if (app == nullptr) {
      throw std::logic_error("MockEngine can only be used with MockApp");
    }

    RegisterMockApplication(app->GetTestDescription());

    Engine::Start();

    LoggerTargetData testing_sink = {
      .sink = {},
      .sink_factory = []() -> spdlog::sink_ptr {
        return std::make_shared<spdlog::sinks::callback_sink_mt>(
          [&](const spdlog::details::log_msg& msg) mutable {
            if (msg.level == spdlog::level::warn) {
              instance->RecordWarning();
            } else if (msg.level == spdlog::level::err) {
              instance->RecordError(ErrorLevel::NON_FATAL);
            } else if (msg.level == spdlog::level::critical) {
              instance->RecordError(ErrorLevel::FATAL);
            }
          }
        );
      },
    };
  }

  void MockEngine::Step() {
    dt = delta.Get();
    AppState::OnEngineTick(dt);

    while (!event_queue.empty()) {
      state->HandleEvent(event_queue.front());
      event_queue.pop();
    }

    if (state->IsFinished()) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      exit_code = AppState::exit_code.value();
    } else {
      state->Step();
    }
  }

  void MockEngine::Stop() {
    Engine::Stop();

    if (run_record.num_errors > 0) {
      run_record.result = RunResult::TEST_FAILURE;
    } else {
      run_record.result = RunResult::TEST_SUCCESS;
    }
    records->Record(TestEngineRecords::RUN_REPORT, run_record);
  }

  Ref<EngineStateMachine> MockEngine::CreateStateMachine() {
    return NewRef<MockEngineStateMachine>(this);
  }

}  // namespace other
