/**
 * \file mock_engine.hpp
 */
#ifndef MOCK_ENGINE_HPP
#define MOCK_ENGINE_HPP

#include <gtest/gtest.h>

#include "core/defines.hpp"
#include "core/config.hpp"

#define TEST_ENGINE_ENV() \
  other::MockEngine::TestEngine();

namespace other {

  class MockEngine : public ::testing::Environment {
    public:
      MockEngine(const Path& configpath);
      virtual ~MockEngine() override;

      static MockEngine* TestEngine();

      virtual void SetUp() override;
      virtual void TearDown() override;

    protected:
      static MockEngine* instance;
      other::ConfigTable config;

      void InitializeEngineSubSystems();
      void ShutdownEngineSubSystems();
  };
      
  class EngineTest : public ::testing::Test {
    public:
  };

} // namespace other


#endif// !MOCK_ENGINE_HPP
