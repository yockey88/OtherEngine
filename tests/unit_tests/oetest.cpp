/**
 * \file unit_tests/oetest.cpp
 **/
#include "unit_tests/oetest.hpp"

#include "core/logger.hpp"
#include "parsing/ini_parser.hpp"

namespace other {

std::optional<ConfigTable> OtherTest::config = {};
std::optional<ConfigTable> OtherTest::stashed_config = {};

CmdLine OtherTest::cmdline = {};
    
void OtherTest::StashConfig() {
  if (!config.has_value()) {
    return;
  }
  stashed_config = *config;
}
    
void OtherTest::ApplyStashedConfig() {
  if (!stashed_config.has_value()) {
    return;
  }

  config = *stashed_config;
}
    
void OtherTest::SetUpTestSuite() {  
  const std::string configpath = "C:/Yock/code/OtherEngine/tests/unit_tests/unittest.other";      
  IniFileParser parser(configpath);
  try {
    config = parser.Parse();
  } catch (IniException& e) {
    FAIL() << "Failed to parse INI file for unit test!\n\t" << e.what();
    return;
  }
}

void OtherTest::TearDownTestSuite() {
}

void OtherTest::SetUp() {
  OpenLog();
}

void OtherTest::TearDown() {
  CloseLog();
}

void OtherTest::OpenLog() {
  if (Logger::IsOpen()) {
    return;
  }

  ASSERT_TRUE(config.has_value());

  /// FIXME: customize config per unit test but somehow not create bajillions of config files
  Logger::Open(*config);
  Logger::Instance()->RegisterThread("Main Test Thread");
}

void OtherTest::CloseLog() {
  Logger::Shutdown();
}

} // namespace other
