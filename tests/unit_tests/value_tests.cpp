/**
 * \file unit_tests/value_tests.cpp
 **/
#include "unit_tests/oetest.hpp"

#include "core/value.hpp"
#include <core/defines.hpp>
#include <gtest.h>

using other::Value;
using other::ValueType;

class ValueTests : public other::OtherTest {
  public:
    Value value;

    void SetUp() override {
      other::OtherTest::SetUp();
      ASSERT_EQ(value.Size() , 0u) <<
        "Value starting test in invalid state!";
      
      ASSERT_EQ(value.Type() , ValueType::EMPTY) <<
        "Value starting test in invalid state!";
    }

    void TearDown() override {
      ASSERT_NO_FATAL_FAILURE(value.Clear());
      other::OtherTest::TearDown();
    }
};

TEST_F(ValueTests , single_val_test1) {
  int32_t num = 333;
  ASSERT_NO_FATAL_FAILURE(value.Set(num));

  ASSERT_EQ(value.Size() , 4);
  ASSERT_EQ(value.Type() , ValueType::INT32);
  EXPECT_EQ(value.Get<int32_t>() , num);
}

TEST_F(ValueTests , array_test1) {
  std::array<size_t , 10> arr = {
    1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10
  };

  EXPECT_NO_FATAL_FAILURE(value.Set<size_t>(arr));
  EXPECT_EQ(value.Size() , sizeof(size_t) * arr.size());
  ASSERT_EQ(value.Type() , ValueType::UINT64);
  ASSERT_EQ(value.NumElements() , 10);

  for (uint32_t i = 0; i < 10; ++i) {
    EXPECT_EQ(value.At<size_t>(i) , arr[i]) <<
      "Failed on .At<> test on step " << i;
  }
}
