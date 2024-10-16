/**
 * \file mock_engine.hpp
 */
#ifndef MOCK_ENGINE_HPP
#define MOCK_ENGINE_HPP

#include <gtest/gtest.h>

#include "core/defines.hpp"
#include "core/config.hpp"
#include "core/engine.hpp"
#include "core/filesystem.hpp"

#include "application/app.hpp"


#define TEST_ENGINE_ENV() \
  other::MockEngine::TestEngine()

namespace other {

  class MockApp : public other::App {
    public:
      MockApp(const CmdLine& cmd_line, const ConfigTable& config) 
        : other::App(cmd_line, config) {}
      virtual ~MockApp() override {}
  };

  constexpr static uint64_t kDefaultTargetHash = other::FNV("Geometry");

  class MockEngine : public ::testing::Environment {
    public:
      MockEngine(CmdLine command_line , const Path& configpath);
      virtual ~MockEngine() override;

      static MockEngine* TestEngine();

      virtual void SetUp() override;
      virtual void TearDown() override;

      Ref<SceneRenderer> GetDefaultSceneRenderer(const uint32_t max_entities = 100);

    protected:
      static MockEngine* instance;
      
      CmdLine command_line;
      ConfigTable config;

      const other::Path engine_core_dir = other::Filesystem::GetEngineCoreDir();
      const other::Path assets_dir = engine_core_dir / "OtherEngine" / "assets";
      const other::Path shader_dir = assets_dir / "shaders";

      Scope<Engine> engine_stub = nullptr;
      Scope<App> mock_application = nullptr;
      Ref<SceneRenderer> default_renderer = nullptr;

      void InitializeEngineSubSystems();
      void ShutdownEngineSubSystems();
  };
      
  class EngineTest : public ::testing::Test {
    public:
    protected:
      void EmptyEventLoop() const;
  };

  Scope<App> NewApp(Engine* engine);

} // namespace other


#endif// !MOCK_ENGINE_HPP
