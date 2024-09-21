/**
 * \file Tests/Native/dotest.hpp
 **/
#ifndef DOTOTHER_DOTEST_HPP
#define DOTOTHER_DOTEST_HPP

#include <gtest/gtest.h>

class DoTest : public ::testing::Test {
  public:
    virtual void SetUp() {}
    
    virtual void TearDown() {}
};

#endif // !DOTOTHER_DOTEST_HPP
