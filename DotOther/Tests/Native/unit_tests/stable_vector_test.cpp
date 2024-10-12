/**
 * \file Native/unit_tests/stable_vector_test.cpp
 **/
#include "core/dotest.hpp"

#include "core/stable_vector.hpp"
#include <gtest.h>

using namespace dotother;

struct A {
  int x;
  int y;
  int z;
};

class StableVectorTests : public DoTest {
  public:
  protected:
    StableVector<float> vec;
    StableVector<A> vec_a;
};

TEST_F(StableVectorTests , default_ctor) {
  EXPECT_EQ(vec.Size() , 0);
  EXPECT_EQ(vec_a.Size() , 0);
}

TEST_F(StableVectorTests , insert_float) {
  auto [idx , val] = vec.EmplaceBackNoLock();
  val = 1.0f;
  ASSERT_EQ(idx , 0);
  ASSERT_EQ(vec.Size() , 1);

  ASSERT_EQ(vec[idx] , 1.0f);
}

TEST_F(StableVectorTests , insert_A) {
  auto [idx , val] = vec_a.EmplaceBackNoLock();
  val.x = 1;
  val.y = 2;
  val.z = 3;
  ASSERT_EQ(idx , 0);
  ASSERT_EQ(vec_a.Size() , 1);

  ASSERT_EQ(vec_a[idx].x , 1);
  ASSERT_EQ(vec_a[idx].y , 2);
  ASSERT_EQ(vec_a[idx].z , 3);
}