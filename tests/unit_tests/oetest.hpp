/**
 * \file unit_tests/oetest.hpp
 **/
#ifndef OETEST_HPP
#define OETEST_HPP

#include <cstdint>

#include <gtest/gtest.h>

#include "core/config.hpp"
#include "parsing/cmd_line_parser.hpp"

namespace other {

class OtherTest : public ::testing::Test {
  public:
    static void SetUpTestSuite(); 
    static void TearDownTestSuite();

    virtual void SetUp() override; 
    virtual void TearDown() override;
    
    static void OpenLog(); 
    static void CloseLog();

  protected:
    static ConfigTable config;
    static ConfigTable stashed_config;

    static CmdLine cmdline;
};

bool CheckNumScripts(uint32_t cs , uint32_t lua , uint32_t python);

} // namespace other

#endif// !OETEST_HPP
