/**
 * \file driver.cpp
 **/
#include "mock_engine.hpp"

#include <iostream>

#include <gtest/gtest.h>

#include "core/errors.hpp"

int main(int argc , char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  const ::testing::Environment* mock_engine = nullptr;

  try {
    /// FIXME: command line argument for config path
    const std::string configpath = "C:/Yock/code/OtherEngine/OtherTestEngine/test_engine.other";
    mock_engine = ::testing::AddGlobalTestEnvironment(new other::MockEngine(configpath));
  } catch (const other::IniException& e) {
    std::cout << "Failed to parse INI file : " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cout << "failed to load mock engine : unknow failure\n";
    return 1;
  }

  return RUN_ALL_TESTS();
}

using other::EngineTest;
TEST_F(EngineTest , example) {
  auto* env = TEST_ENGINE();
}

