/**
 * \file mock_engine.hpp
 */
#ifndef MOCK_ENGINE_HPP
#define MOCK_ENGINE_HPP

#include <source_location>

#include <gtest/gtest.h>

#include "statistics/test_engine_records.hpp"
#include "testing_core/errors.hpp"
#include "testing_core/run_report.hpp"
#include "testing_core/test.hpp"

#include "engine/engine.hpp"
#include "engine/engine_states.hpp"

#include "application/app.hpp"

namespace other {

  class MockEngine;

  class MockEngineStateMachine : public EngineStateMachine {
   public:
    MockEngineStateMachine(MockEngine* engine)
        : EngineStateMachine((Engine*)engine), mock_engine(engine) {}
    virtual ~MockEngineStateMachine() override {}

    MockEngine* mock_engine = nullptr;
  };

  class MockEngine : public Engine {
   public:
    MockEngine(CmdLine cmd_line, std::string main_thread_name);
    virtual ~MockEngine() override;

    static MockEngine* TestEngine();

    void RegisterMockApplication(const TestDescription& description);

    void RecordError(ErrorLevel level, const std::string_view msg, std::source_location loc);
    void RecordWarning(const std::string_view msg, std::source_location loc);

    void RecordError(const OtherTestEngineError& error);
    void RecordWarning(const OtherTestEngineError& error);

    virtual void Run() override;

    void InitializeTest();
    void UpdateTest();
    void RenderTest();

   protected:
    static MockEngine* instance;

    size_t num_warnings_encountered = 0;
    size_t num_errors_encountered = 0;
    Scope<TestEngineRecords> records = nullptr;

    RunReport run_record;

    virtual void Start() override;
    virtual void Step() override;
    virtual void Stop() override;

    Ref<EngineStateMachine> CreateStateMachine() override;
  };

  class MockApp : public other::App {
   public:
    MockApp(const CmdLine& cmd_line, const ConfigTable& config)
        : other::App(cmd_line, config) {}
    virtual ~MockApp() override {}

    virtual TestDescription GetTestDescription() = 0;

    // Ref<AssetHandler> CreateAssetHandler() override;
    // Ref<SceneRenderer> CreateSceneRenderer() override;
  };

}  // namespace other

#endif  // !MOCK_ENGINE_HPP
