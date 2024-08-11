/**
 * \file test/unit-tests/oetest.hpp
 **/
#ifndef OETEST_HPP
#define OETEST_HPP

#include <gtest/gtest.h>

#include "core/logger.hpp"
#include "core/config.hpp"
#include "core/errors.hpp"

#include "parsing/ini_parser.hpp"

namespace other {

class OtherTest : public ::testing::Test {
  public:
    static void SetUpTestSuite() {

    }
    
    static void TearDownTestSuite() {
      const std::string configpath = "C:/Yock/code/OtherEngine/tests/unit_tests/unittest.other";      
      other::IniFileParser parser(configpath);
      try {
        config = parser.Parse();
      } catch (other::IniException& e) {
        FAIL() << "Failed to parse INI file for unit test!\n";
        return;
      }
    }

    virtual void SetUp() override {
      /// FIXME: customize config per unit test but somehow not create bajillions of config files
      other::Logger::Open(config);
      other::Logger::Instance()->RegisterThread("Main Test Thread");
    }
    
    virtual void TearDown() override {
      other::Logger::Shutdown();
    }

  protected:
    static other::ConfigTable config;
};

} // namespace other

#endif// !OETEST_HPP
