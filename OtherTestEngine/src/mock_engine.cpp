/**
 * \file mock_engine.cpp
 **/
#include "mock_engine.hpp"

#include <source_location>
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

  void MockEngine::RecordError(ErrorLevel level, const std::string_view msg, std::source_location loc) {
    run_record.num_errors++;
    records->Record(TestEngineRecords::ERROR_DESCRIPTION, OtherTestEngineError(level, msg, loc));
  }

  void MockEngine::RecordWarning(const std::string_view msg, std::source_location loc) {
    run_record.num_warnings++;
    records->Record(TestEngineRecords::ERROR_DESCRIPTION, OtherTestEngineError(ErrorLevel::WARNING, msg, loc));
  }

  void MockEngine::RecordError(const OtherTestEngineError& error) {
    run_record.num_errors++;
    records->Record(TestEngineRecords::ERROR_DESCRIPTION, error);
  }

  void MockEngine::RecordWarning(const OtherTestEngineError& error) {
    run_record.num_warnings++;
    records->Record(TestEngineRecords::ERROR_DESCRIPTION, error);
  }

  void MockEngine::Run() {
    Start();

    OE_INFO("Running");
    do {
      try {
        Step();
      } catch (OtherTestEngineError& e) {
        e.num = run_record.num_errors++;

        records->Record(TestEngineRecords::ERROR_DESCRIPTION, e);
        EngineEvent(EngineStateEvent::ENGINE_FAILURE);
      } catch (std::logic_error& e) {
        std::string str = fmtstr("Did you attempt to access the test engine before initializing it? {}", e.what());
        OtherTestEngineError err(ErrorLevel::FATAL, str);
        err.num = run_record.num_errors++;

        records->Record(TestEngineRecords::ERROR_DESCRIPTION, err);
        EngineEvent(EngineStateEvent::ENGINE_FAILURE);
      } catch (...) {
        OtherTestEngineError err(ErrorLevel::FATAL, "Unknown exception caught at top level");
        err.num = run_record.num_errors++;

        records->Record(TestEngineRecords::ERROR_DESCRIPTION, err);
        EngineEvent(EngineStateEvent::ENGINE_FAILURE);
      }
    } while (!exit_code.has_value());

    OE_ASSERT(exit_code.has_value(), "Driver did not set exit code");
    Stop();
  }

  void MockEngine::InitializeTest() {
    MockApp& app = AppState::AppHandle<MockApp>();
    RegisterMockApplication(app.GetTestDescription());
    EngineEvent(EngineStateEvent::TEST_START);
  }

  void MockEngine::UpdateTest() {
  }

  void MockEngine::RenderTest() {
  }

  namespace {

    void RecordLog(const spdlog::details::log_msg& msg) {
      std::string msg_str = fmtstr("{}", msg.payload);

      ErrorMarker marker(msg.source.line, 0, "<unknown>");
      if (!msg.source.empty()) {
        marker = ErrorMarker(msg.source.line, 0, msg.source.filename);
      }

      if (msg.level == spdlog::level::warn) {
        OtherTestEngineError err(ErrorLevel::WARNING, msg_str, marker);
        MockEngine::TestEngine()->RecordWarning(err);
      } else if (msg.level == spdlog::level::err) {
        OtherTestEngineError err(ErrorLevel::NON_FATAL, msg_str, marker);
        MockEngine::TestEngine()->RecordError(err);
      } else if (msg.level == spdlog::level::critical) {
        OtherTestEngineError err(ErrorLevel::FATAL, msg_str, marker);
        MockEngine::TestEngine()->RecordError(err);
      }
    }

    spdlog::sink_ptr CreateTestingSink() {
      return std::make_shared<spdlog::sinks::callback_sink_mt>(&RecordLog);
    }

  }  // anonymous namespace

  void MockEngine::Start() {
    instance = this;
    records = NewScope<TestEngineRecords>();

    Engine::Start();

    LoggerTargetData log_intercept = {
      .sink = {
        .sink_name = "log-intercept-sink",
        .sink_pattern = "%v",
        .level = spdlog::level::trace,
      },
      .sink_factory = &CreateTestingSink,
    };
    Logger::Instance()->RegisterTarget(log_intercept);
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
    records = nullptr;
  }

  Ref<EngineStateMachine> MockEngine::CreateStateMachine() {
    return NewRef<MockEngineStateMachine>(this);
  }

}  // namespace other
