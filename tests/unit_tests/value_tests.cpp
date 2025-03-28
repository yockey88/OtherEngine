/**
 * \file unit_tests/value_tests.cpp
 **/
#include <core/defines.hpp>
#include <gtest.h>

#include "core/value.hpp"

#include "unit_tests/oetest.hpp"

using other::Value;
using other::ValueType;

class ValueTests : public other::OtherTest {
 public:
  Value value;

  void SetUp() override {
    other::OtherTest::SetUp();
    ASSERT_EQ(value.GetSize(), 0u) << "Value starting test in invalid state!";

    ASSERT_EQ(value.GetType(), ValueType::EMPTY_TYPE) << "Value starting test in invalid state!";
  }

  void TearDown() override {
    ASSERT_NO_FATAL_FAILURE(value.Clear());
    other::OtherTest::TearDown();
  }
};

TEST_F(ValueTests, single_val_test1) {
  int32_t num = 333;
  ASSERT_NO_FATAL_FAILURE(value = num);

  ASSERT_EQ(value.GetSize(), 4);
  ASSERT_EQ(value.GetType(), ValueType::INT32);
  EXPECT_EQ(value.Get<int32_t>(), num);
}

TEST_F(ValueTests, array_test1) {
  std::array<size_t, 10> arr = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  };

  EXPECT_NO_FATAL_FAILURE(value = arr);
  EXPECT_EQ(value.GetSize(), sizeof(size_t) * arr.size());
  ASSERT_EQ(value.GetType(), ValueType::UINT64);
  ASSERT_EQ(value.NumElements(value.GetType()), 10);

  // for (uint32_t i = 0; i < 10; ++i) {
  //   EXPECT_EQ(value.Get<size_t>(i), arr[i]) << "Failed on .At<> test on step " << i;
  // }
}

TEST_F(ValueTests, string_test1) {
  std::string str = "Hello, World!";
  ASSERT_NO_FATAL_FAILURE(value = str);

  ASSERT_EQ(value.GetSize(), str.length());
  ASSERT_EQ(value.GetType(), ValueType::STRING);
  EXPECT_EQ(value.Get<std::string>(), str);
}
