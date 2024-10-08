/**
 * \file unit_tests/vecmath_tests.cpp
 **/
#include "oetest.hpp"
#include "math/vecmath.hpp"

using namespace other;
using other::EpsilonSubtract;
using other::EpsilonAdd;
using other::EpsilonDifference;
using other::EpsilonLt;

class VecmathTests : public other::OtherTest {
  public:
};

TEST_F(VecmathTests, epsilon_clamp) {
  ASSERT_EQ(EpsilonClamp(0.0f), 0.0f);
  ASSERT_EQ(EpsilonClamp(0.1f), 0.1f);
  ASSERT_EQ(EpsilonClamp(-0.1f), -0.1f);
  ASSERT_EQ(EpsilonClamp(0.0001f), 0.0001f);
  ASSERT_EQ(EpsilonClamp(-0.0001f), -0.0001f);
  ASSERT_EQ(EpsilonClamp(0.00001f), 0.00001f);
  ASSERT_EQ(EpsilonClamp(-0.00001f), -0.00001f);
  ASSERT_EQ(EpsilonClamp(0.000001f), 0.000001f);
  ASSERT_EQ(EpsilonClamp(-0.000001f), -0.000001f);
  ASSERT_EQ(EpsilonClamp(0.0000001f), 0);
  ASSERT_EQ(EpsilonClamp(-0.0000001f), 0);
  ASSERT_EQ(EpsilonClamp(std::numeric_limits<float>::epsilon()), 0);
  ASSERT_EQ(EpsilonClamp(-std::numeric_limits<float>::epsilon()), 0);
}

TEST_F(VecmathTests, epsilon_zero) {
  ASSERT_EQ(EpsilonZero(0.0f), true);
  ASSERT_EQ(EpsilonZero(0.1f), false);
  ASSERT_EQ(EpsilonZero(-0.1f), false);
  ASSERT_EQ(EpsilonZero(0.0001f), false);
  ASSERT_EQ(EpsilonZero(-0.0001f), false);
  ASSERT_EQ(EpsilonZero(0.00001f), false);
  ASSERT_EQ(EpsilonZero(-0.00001f), false);
  ASSERT_EQ(EpsilonZero(0.000001f), false);
  ASSERT_EQ(EpsilonZero(-0.000001f), false);
  ASSERT_EQ(EpsilonZero(0.0000001f), true);
  ASSERT_EQ(EpsilonZero(-0.0000001f), true);
  ASSERT_EQ(EpsilonZero(std::numeric_limits<float>::epsilon()), true);
  ASSERT_EQ(EpsilonZero(-std::numeric_limits<float>::epsilon()), true);
}

TEST_F(VecmathTests, epsilon_abs) {
  ASSERT_EQ(EpsilonAbs(0.0f), 0.0f);
  ASSERT_EQ(EpsilonAbs(0.1f), 0.1f);
  ASSERT_EQ(EpsilonAbs(-0.1f), 0.1f);
  ASSERT_EQ(EpsilonAbs(0.0001f), 0.0001f);
  ASSERT_EQ(EpsilonAbs(-0.0001f), 0.0001f);
  ASSERT_EQ(EpsilonAbs(0.00001f), 0.00001f);
  ASSERT_EQ(EpsilonAbs(-0.00001f), 0.00001f);
  ASSERT_EQ(EpsilonAbs(0.000001f), 0.000001f);
  ASSERT_EQ(EpsilonAbs(-0.000001f), 0.000001f);
}

TEST_F(VecmathTests, epsilon_subtract) {
  ASSERT_EQ(EpsilonSubtract(0.0f, 0.1f), -0.1f);
  ASSERT_EQ(EpsilonSubtract(0.0f, -0.1f), 0.1f);
  ASSERT_EQ(EpsilonSubtract(0.0f, 0.0001f), -0.0001f);
  ASSERT_EQ(EpsilonSubtract(0.0f, -0.0001f), 0.0001f);
  ASSERT_EQ(EpsilonSubtract(0.0f, 0.00001f), -0.00001f);
  ASSERT_EQ(EpsilonSubtract(0.0f, -0.00001f), 0.00001f);
  ASSERT_EQ(EpsilonSubtract(0.0f, 0.000001f), -0.000001f);
  ASSERT_EQ(EpsilonSubtract(0.0f, -0.000001f), 0.000001f);
  ASSERT_EQ(EpsilonSubtract(0.0f, 0.0000001f), 0);
  ASSERT_EQ(EpsilonSubtract(0.0f, -0.0000001f), 0);
  ASSERT_EQ(EpsilonSubtract(0.0f, std::numeric_limits<float>::epsilon()), 0);
  ASSERT_EQ(EpsilonSubtract(0.0f, -std::numeric_limits<float>::epsilon()), 0);
}

TEST_F(VecmathTests, epsilon_add) {
  ASSERT_EQ(EpsilonAdd(0.0f, 0.1f), 0.1f);
  ASSERT_EQ(EpsilonAdd(0.0f, -0.1f), -0.1f);
  ASSERT_EQ(EpsilonAdd(0.0f, 0.0001f), 0.0001f);
  ASSERT_EQ(EpsilonAdd(0.0f, -0.0001f), -0.0001f);
  ASSERT_EQ(EpsilonAdd(0.0f, 0.00001f), 0.00001f);
  ASSERT_EQ(EpsilonAdd(0.0f, -0.00001f), -0.00001f);
  ASSERT_EQ(EpsilonAdd(0.0f, 0.000001f), 0.000001f);
  ASSERT_EQ(EpsilonAdd(0.0f, -0.000001f), -0.000001f);
  ASSERT_EQ(EpsilonAdd(0.0f, 0.0000001f), 0);
  ASSERT_EQ(EpsilonAdd(0.0f, -0.0000001f), 0);
  ASSERT_EQ(EpsilonAdd(0.0f, std::numeric_limits<float>::epsilon()), 0);
  ASSERT_EQ(EpsilonAdd(0.0f, -std::numeric_limits<float>::epsilon()), 0);
}

TEST_F(VecmathTests, epsilon_difference) {
  ASSERT_EQ(EpsilonDifference(0.0f, 0.1f), 0.1f);
  ASSERT_EQ(EpsilonDifference(0.0f, -0.1f), 0.1f);
  ASSERT_EQ(EpsilonDifference(0.0f, 0.0001f), 0.0001f);
  ASSERT_EQ(EpsilonDifference(0.0f, -0.0001f), 0.0001f);
  ASSERT_EQ(EpsilonDifference(0.0f, 0.00001f), 0.00001f);
  ASSERT_EQ(EpsilonDifference(0.0f, -0.00001f), 0.00001f);
  ASSERT_EQ(EpsilonDifference(0.0f, 0.000001f), 0.000001f);
  ASSERT_EQ(EpsilonDifference(0.0f, -0.000001f), 0.000001f);
  ASSERT_EQ(EpsilonDifference(0.0f, 0.0000001f), 0);
  ASSERT_EQ(EpsilonDifference(0.0f, -0.0000001f), 0);
  ASSERT_EQ(EpsilonDifference(0.0f, std::numeric_limits<float>::epsilon()), 0);
  ASSERT_EQ(EpsilonDifference(0.0f, -std::numeric_limits<float>::epsilon()), 0);
}

TEST_F(VecmathTests, epsilon_lt) {
  ASSERT_EQ(EpsilonLt(0.0f, 0.1f), true);
  ASSERT_EQ(EpsilonLt(0.1f, 0.0f), false);
  ASSERT_EQ(EpsilonLt(0.0f, -0.1f), false);
  ASSERT_EQ(EpsilonLt(-0.1f, 0.0f), true);
  ASSERT_EQ(EpsilonLt(0.0f, 0.0001f), true);
  ASSERT_EQ(EpsilonLt(0.0001f, 0.0f), false);
  ASSERT_EQ(EpsilonLt(0.0f, -0.0001f), false);
  ASSERT_EQ(EpsilonLt(-0.0001f, 0.0f), true);
  ASSERT_EQ(EpsilonLt(0.0f, 0.00001f), true);
  ASSERT_EQ(EpsilonLt(0.00001f, 0.0f), false);
  ASSERT_EQ(EpsilonLt(0.0f, -0.00001f), false);
  ASSERT_EQ(EpsilonLt(-0.00001f, 0.0f), true);
  ASSERT_EQ(EpsilonLt(0.0f, 0.000001f), true);
  ASSERT_EQ(EpsilonLt(0.000001f, 0.0f), false);
  ASSERT_EQ(EpsilonLt(0.0f, -0.000001f), false);
  ASSERT_EQ(EpsilonLt(-0.000001f, 0.0f), true);
  ASSERT_EQ(EpsilonLt(0.0f, 0.0000001f), false);
  ASSERT_EQ(EpsilonLt(0.0000001f, 0.0f), false);
  ASSERT_EQ(EpsilonLt(0.0f, -0.0000001f), false);
  ASSERT_EQ(EpsilonLt(-0.0000001f, 0.0f), false);
  ASSERT_EQ(EpsilonLt(0.0f, std::numeric_limits<float>::epsilon()), false);
  ASSERT_EQ(EpsilonLt(std::numeric_limits<float>::epsilon(), 0.0f), false);
  ASSERT_EQ(EpsilonLt(0.0f, -std::numeric_limits<float>::epsilon()), false);
  ASSERT_EQ(EpsilonLt(-std::numeric_limits<float>::epsilon(), 0.0f), false);
}

TEST_F(VecmathTests, vec_diff) {
  glm::vec2 v1(1.0f, 2.0f);
  glm::vec2 v2(1.0f, 2.0000001f);
  glm::vec2 expected_diff(0.0f, 0.0f);
  ASSERT_EQ(vec2_diff(v1, v2), expected_diff);
}

TEST_F(VecmathTests, vec3_diff) {
  glm::vec3 v1(1.0f, 2.0f, 3.0f);
  glm::vec3 v2(1.0f, 2.0000001f, 3.0f);
  glm::vec3 expected_diff(0.0f, 0.0f, 0.0f);
  ASSERT_EQ(vec3_diff(v1, v2), expected_diff);
}

TEST_F(VecmathTests, vec4_diff) {
  glm::vec4 v1(1.0f, 2.0f, 3.0f, 4.0f);
  glm::vec4 v2(1.0f, 2.0000001f, 3.0f, 4.0f);
  glm::vec4 expected_diff(0.0f, 0.0f, 0.0f, 0.0f);
  ASSERT_EQ(vec4_diff(v1, v2), expected_diff);
}

TEST_F(VecmathTests, vec2_sum) {
  glm::vec2 v1(1.0f, 2.0f);
  glm::vec2 v2(1.0f, 2.0000001f);
  glm::vec2 expected_sum(2.0f, 4.0f);
  ASSERT_EQ(vec2_sum(v1, v2), expected_sum);
}

TEST_F(VecmathTests, vec3_sum) {
  glm::vec3 v1(1.0f, 2.0f, 3.0f);
  glm::vec3 v2(1.0f, 2.0000001f, 3.0f);
  glm::vec3 expected_sum(2.0f, 4.0f, 6.0f);
  ASSERT_EQ(vec3_sum(v1, v2), expected_sum);
}

TEST_F(VecmathTests, vec4_sum) {
  glm::vec4 v1(1.0f, 2.0f, 3.0f, 4.0f);
  glm::vec4 v2(1.0f, 2.0000001f, 3.0f, 4.0f);
  glm::vec4 expected_sum(2.0f, 4.0f, 6.0f, 8.0f);
  ASSERT_EQ(vec4_sum(v1, v2), expected_sum);
}