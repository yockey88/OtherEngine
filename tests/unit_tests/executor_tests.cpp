/**
 * \file executor_tests.cpp
 **/
#include "core/filesystem.hpp"
#include "environment/command_executor.hpp"

#include "application/app_state.hpp"

#include "physics/phyics_engine.hpp"
#include "rendering/renderer.hpp"
#include "scripting/script_engine.hpp"

#include "oetest.hpp"

using namespace other;
class EnvExecutorTests : public OtherTest {
 public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();

  //// no default behavior for now
  virtual void SetUp() override {}
  virtual void TearDown() override {}
};

TEST_F(EnvExecutorTests, simple_command) {
  Memory memory;
  CommandParser parser(memory);

  std::string command = "load scene renderless_scene;";

  CommandBlock block = parser.ParseBlock(command);
  ASSERT_EQ(block.name, "<command-block-0>");
  ASSERT_EQ(block.command_queue.size(), 2u);

  Instructions instructions = {
    .module_name = "test-command",
    .blocks = {
      { FNV("test-command"), block } },
  };

  ExitCode ec = FAILURE;

  Executor exe(memory, instructions);
  ASSERT_NO_THROW(ec = exe.Execute());
  ASSERT_EQ(ec, SUCCESS);
}

void EnvExecutorTests::SetUpTestSuite() {
  ConfigTable test_config;
  test_config.Add("project", "working-directory", "C:/Yock/code/OtherEngine/tests");
  test_config.Add("project", "bin-dir", "C:/Yock/code/OtherEngine/bin/Debug");
  test_config.Add("project", "scenes-dir", "C:/Yock/code/OtherEngine/tests/sandbox/scenes");
  test_config.Add("log", "console-level", "debug", true);
  test_config.Add("log", "file-level", "trace", true);
  test_config.Add("log", "path", "logs/env-executor-test.log", true);
  Logger::Open(test_config);
  Logger::Instance()->RegisterThread("Script Engine Test Main Thread");

  Filesystem::Initialize(cmdline, test_config);
  AppState::mode = EngineMode::RUNTIME;
  AppState::Initialize(cmdline, test_config);
  // Renderer::Initialize(test_config);
  ScriptEngine::Initialize(test_config);
  PhysicsEngine::Initialize(test_config);
}

void EnvExecutorTests::TearDownTestSuite() {
  ASSERT_NO_FATAL_FAILURE(PhysicsEngine::Shutdown());
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::Shutdown());
  ASSERT_NO_FATAL_FAILURE(AppState::Shutdown());
  ASSERT_NO_FATAL_FAILURE(CloseLog());
}
