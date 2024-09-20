/**
 * \file Tests/Native/dotest.hpp
 **/
#ifndef DOTOTHER_DOTEST_HPP
#define DOTOTHER_DOTEST_HPP

#include <memory>

#include <gtest/gtest.h>

template <typename T>
using Scope = std::unique_ptr<T>;

class DoTest : public ::testing::Test {
  public:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif // !DOTOTHER_DOTEST_HPP