/**
 * \file unit_test/bvh_tests.cpp
 **/
#include <glm/glm.hpp>

#include "core/config.hpp"
#include "core/logger.hpp"

#include "scene/bvh.hpp"
#include "scene/bvh_node.hpp"

#include "oetest.hpp"

using namespace other;

class BvhTests : public other::OtherTest {
 public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();

  virtual void SetUp() override {}
  virtual void TearDown() override {}

 protected:
  constexpr static glm::vec3 kOrigin{0.f};
};

TEST_F(BvhTests, normal_bvh_basic) {
  using BVH = Bvh<2, HLBVH>;
  using Node = BvhNode<2, HLBVH>;

  Ref<BVH> bvh = NewRef<BVH>(glm::vec3{0.f});
  ASSERT_NE(bvh, nullptr);
  ASSERT_EQ(bvh->NumNodes(), 1);
  ASSERT_EQ(bvh->Depth(), 0);

  {
    Node& n = bvh->GetSpace();
    ASSERT_EQ(n.GetMaxDepth(), 0);
    ASSERT_EQ(n.tree_index, 0);
    ASSERT_EQ(n.partition_index, 0);
    ASSERT_EQ(n.partition_location, 0b000);
    ASSERT_EQ(n.Min(), glm::vec3(-0.f));
    ASSERT_EQ(n.Max(), glm::vec3(0.f));
  }

  bvh->Subdivide({1.f, 1.f, 1.f}, 1);
  ASSERT_EQ(bvh->NumNodes(), 1 + 2);
}

void BvhTests::SetUpTestSuite() {
  ConfigTable test_config = ConfigTable{};
  test_config.Add("log", "console-level", "trace", true);
  test_config.Add("log", "file-level", "trace", true);
  test_config.Add("log", "path", "logs/bvh-tests.log", true);

  Logger::Open(test_config);
  Logger::Instance()->RegisterThread("Script Scene Integration Test Main Thread");
}

void BvhTests::TearDownTestSuite() {
  CloseLog();
}
