/**
 * \file unit_test/octree_tests.cpp
 **/
#include <glm/glm.hpp>

#include <gtest.h>

#include "core/config.hpp"
#include "core/logger.hpp"

#include "ecs/entity.hpp"
#include "scene/bvh_node.hpp"
#include "scene/octree.hpp"

#include "oetest.hpp"

class OctreeTests : public other::OtherTest {
 public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();

  virtual void SetUp() override {}
  virtual void TearDown() override {}

 protected:
  constexpr static glm::vec3 kOrigin{0.f};
};

using other::fmtstr;

using other::NewRef;
using other::Ref;

using other::ConfigTable;
using other::Logger;

using other::Octant;
using other::Octree;

using other::kLocationIndex;
using other::kOctantLocations;

TEST_F(OctreeTests, validate_arrays) {
  ASSERT_EQ(kLocationIndex.size(), 8);

  ASSERT_EQ(kLocationIndex.at(0b000), 0);
  ASSERT_EQ(kLocationIndex.at(0b100), 1);
  ASSERT_EQ(kLocationIndex.at(0b110), 2);
  ASSERT_EQ(kLocationIndex.at(0b010), 3);
  ASSERT_EQ(kLocationIndex.at(0b001), 4);
  ASSERT_EQ(kLocationIndex.at(0b101), 5);
  ASSERT_EQ(kLocationIndex.at(0b111), 6);
  ASSERT_EQ(kLocationIndex.at(0b011), 7);

  ASSERT_EQ(kOctantLocations[0], 0b000);
  ASSERT_EQ(kOctantLocations[1], 0b100);
  ASSERT_EQ(kOctantLocations[2], 0b110);
  ASSERT_EQ(kOctantLocations[3], 0b010);
  ASSERT_EQ(kOctantLocations[4], 0b001);
  ASSERT_EQ(kOctantLocations[5], 0b101);
  ASSERT_EQ(kOctantLocations[6], 0b111);
  ASSERT_EQ(kOctantLocations[7], 0b011);
}

TEST_F(OctreeTests, depth_0) {
  constexpr static uint32_t kDepth = 0;

  Ref<Octree> tree = NewRef<Octree>(glm::vec3{0.f, 0.f, 0.f});
  ASSERT_NE(tree, nullptr);

  tree->Subdivide({1.f, 1.f, 1.f}, kDepth);
  ASSERT_EQ(tree->NumNodes(), 1);
  ASSERT_EQ(tree->Depth(), kDepth);

  Octant& space = tree->GetSpace();
  ASSERT_TRUE(space.IsLeaf());
  ASSERT_EQ(space.GetMaxDepth(), 0);
  ASSERT_EQ(space.tree_index, 0);
  ASSERT_EQ(space.partition_index, 0);
  ASSERT_EQ(space.partition_location, 0b000);
  ASSERT_EQ(space.Min(), glm::vec3(-0.5f))
    << fmtstr("Expected : {} , Actual : {}", glm::vec3(-0.5f), space.Min());
  ASSERT_EQ(space.Max(), glm::vec3(0.5f))
    << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.5f), space.Max());
}

TEST_F(OctreeTests, depth_1) {
  constexpr static uint32_t kDepth = 1;

  Ref<Octree> tree = NewRef<Octree>(glm::vec3{0.f, 0.f, 0.f});
  ASSERT_NE(tree, nullptr);
  tree->Subdivide({1.f, 1.f, 1.f}, kDepth);

  ASSERT_EQ(tree->NumNodes(), 1 + 8);
  ASSERT_EQ(tree->Depth(), kDepth);

  Octant& space = tree->GetSpace();
  ASSERT_FALSE(space.IsLeaf());
  ASSERT_EQ(space.GetMaxDepth(), 1);
  ASSERT_EQ(space.tree_index, 0);
  ASSERT_EQ(space.partition_index, 0);
  ASSERT_EQ(space.partition_location, 0b000);

  for (auto& c : space.Children()) {
    ASSERT_NE(c, nullptr);
    ASSERT_TRUE(c->IsLeaf());
    ASSERT_EQ(c->GetMaxDepth(), 0);

    auto& idx = kLocationIndex.at(c->partition_location);
    ASSERT_EQ(c->partition_index, idx);
    ASSERT_EQ(c->partition_location, kOctantLocations[c->partition_index]);

    if (c->partition_location == 0b000) {
      ASSERT_EQ(c->Min(), glm::vec3(0.f))
        << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.0f), c->Min());
      ASSERT_EQ(c->Max(), glm::vec3(0.5f))
        << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.5f), c->Max());
    } else if (c->partition_location == 0b111) {
      ASSERT_EQ(c->Min(), glm::vec3(-0.5f))
        << fmtstr("Expected : {} , Actual : {}", glm::vec3(-0.5f), c->Min());
      ASSERT_EQ(c->Max(), glm::vec3(0.f))
        << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.0f), c->Max());
    }
  }
}

TEST_F(OctreeTests, higher_res_2) {
  constexpr static uint32_t kDepth = 2;

  Ref<Octree> tree = NewRef<Octree>(glm::vec3{0.f, 0.f, 0.f});
  ASSERT_NE(tree, nullptr);
  tree->Subdivide({1.f, 1.f, 1.f}, kDepth);

  {
    Octant& space = tree->GetSpace();
    ASSERT_FALSE(space.IsLeaf());
    ASSERT_EQ(space.GetMaxDepth(), 2);
    ASSERT_EQ(space.tree_index, 0);
    ASSERT_EQ(space.partition_index, 0);
    ASSERT_EQ(space.partition_location, 0b000);

    for (auto& c : space.Children()) {
      ASSERT_NE(c, nullptr);
      ASSERT_FALSE(c->IsLeaf());
      ASSERT_EQ(c->GetMaxDepth(), 1);
      ASSERT_EQ(c->partition_index, kLocationIndex.at(c->partition_location));
      ASSERT_EQ(c->partition_location, other::kOctantLocations[c->partition_index]);
      ASSERT_EQ(c->GetMaxDepth(), 1);
      ASSERT_EQ(c->partition_index, kLocationIndex.at(c->partition_location));
      ASSERT_EQ(c->partition_location, other::kOctantLocations[c->partition_index]);

      auto& idx = kLocationIndex.at(c->partition_location);
      ASSERT_EQ(c->partition_index, idx);
      if (c->partition_location == 0b000) {
        ASSERT_EQ(c->Min(), glm::vec3(0.f))
          << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.0f), c->Min());
        ASSERT_EQ(c->Max(), glm::vec3(0.5f))
          << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.5f), c->Max());
      }

      for (auto& gc : c->Children()) {
        ASSERT_NE(gc, nullptr);
        ASSERT_TRUE(gc->IsLeaf());
        ASSERT_EQ(gc->GetMaxDepth(), 0);
        ASSERT_EQ(gc->partition_index, kLocationIndex.at(gc->partition_location));
        ASSERT_EQ(gc->partition_location, other::kOctantLocations[gc->partition_index]);
      }
    }
  }

  {
    Octant& posxyz = tree->FindNode(0b000, 1);
    ASSERT_FALSE(posxyz.IsLeaf());
    ASSERT_EQ(posxyz.GetMaxDepth(), 1);
    ASSERT_EQ(posxyz.partition_location, 0b000)
      << fmtstr("Expected : {:>03b} , Actual : {:>03b}", 0b111, posxyz.partition_location);
    ASSERT_EQ(posxyz.Min(), glm::vec3(0.0f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.0f), posxyz.Min());
    ASSERT_EQ(posxyz.Max(), glm::vec3(0.5f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.5f), posxyz.Max());

    auto idx = kLocationIndex.at(0b000);
    ASSERT_EQ(posxyz.partition_index, idx);
  }

  {
    Octant& negxyz = tree->FindNode(0b111, 1);
    OE_DEBUG(" > Found octant {}", negxyz);
    ASSERT_EQ(negxyz.GetMaxDepth(), 1);
    ASSERT_EQ(negxyz.partition_location, 0b111)
      << fmtstr("Expected : {:>03b} , Actual : {:>03b}", 0b111, negxyz.partition_location);
    ASSERT_EQ(negxyz.Min(), glm::vec3(-0.5f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(-0.5f), negxyz.Min());
    ASSERT_EQ(negxyz.Max(), glm::vec3(0.f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.f), negxyz.Max());

    auto idx = kLocationIndex.at(0b111);
    ASSERT_EQ(negxyz.partition_index, idx);
  }

  {
    Octant& negxposy = tree->FindNode(0b001, 1);
    ASSERT_EQ(negxposy.Children().size(), 8);
    ASSERT_EQ(negxposy.GetMaxDepth(), 1);
    ASSERT_EQ(negxposy.partition_location, 0b001)
      << fmtstr("Expected : {:>03b} , Actual : {:>03b}", 0b001, negxposy.partition_location);
    ASSERT_EQ(negxposy.Min(), glm::vec3(0.0f, 0.0f, -0.5f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.0f, 0.0f, -0.5f), negxposy.Min());
    ASSERT_EQ(negxposy.Max(), glm::vec3(0.5f, 0.5f, 0.0f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.5f, 0.5f, 0.0f), negxposy.Max());

    auto idx = kLocationIndex.at(0b001);
    ASSERT_EQ(negxposy.partition_index, idx);
  }

  {
    Octant& posxnegy = tree->FindNode(0b110, 1);
    ASSERT_EQ(posxnegy.GetMaxDepth(), 1);
    ASSERT_EQ(posxnegy.partition_location, 0b110)
      << fmtstr("Expected : {:>03b} , Actual : {:>03b}", 0b110, posxnegy.partition_location);
    ASSERT_EQ(posxnegy.Min(), glm::vec3(-0.5f, -0.5f, 0.0f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(-0.5f, -0.5f, 0.0f), posxnegy.Min());
    ASSERT_EQ(posxnegy.Max(), glm::vec3(0.0f, 0.0f, 0.5f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.0f, 0.0f, 0.5f), posxnegy.Max());

    auto idx = kLocationIndex.at(0b110);
    ASSERT_EQ(posxnegy.partition_index, idx);
  }

  auto& space = tree->GetSpace();
  ASSERT_EQ(space.Min(), space.FindFurthestNode(0b111).Min());
  ASSERT_EQ(space.Max(), space.FindFurthestNode(0b000).Max());
}

TEST_F(OctreeTests, higher_res_3) {
  constexpr static uint32_t kDepth = 3;

  Ref<Octree> tree = NewRef<Octree>(glm::vec3{0.f, 0.f, 0.f});
  ASSERT_NE(tree, nullptr);
  tree->Subdivide({1.f, 1.f, 1.f}, kDepth);
  {
    Octant& space = tree->GetSpace();
    ASSERT_EQ(space.GetMaxDepth(), 3);
    ASSERT_EQ(space.tree_index, 0);
    ASSERT_EQ(space.partition_index, 0);
    ASSERT_EQ(space.partition_location, 0b000);

    for (auto& c : space.Children()) {
      ASSERT_NE(c, nullptr);
      ASSERT_FALSE(c->IsLeaf());
      ASSERT_EQ(c->GetMaxDepth(), 2);

      for (auto& gc : c->Children()) {
        ASSERT_NE(gc, nullptr);
        ASSERT_FALSE(gc->IsLeaf());
        ASSERT_EQ(gc->GetMaxDepth(), 1);

        for (auto& ggc : gc->Children()) {
          ASSERT_NE(ggc, nullptr);
          ASSERT_TRUE(ggc->IsLeaf());
          ASSERT_EQ(ggc->GetMaxDepth(), 0);
        }
      }
    }
  }

  {
    Octant& posxyz = tree->FindNode(0b000, 1);
    ASSERT_EQ(posxyz.GetMaxDepth(), 2);
    ASSERT_EQ(posxyz.partition_location, 0b000);

    auto idx = kLocationIndex.at(0b000);
    ASSERT_EQ(posxyz.partition_index, idx);
    ASSERT_EQ(posxyz.Min(), glm::vec3(0.0f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.0f), posxyz.Min());
    ASSERT_EQ(posxyz.Max(), glm::vec3(0.5f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.5f), posxyz.Max());
  }

  {
    Octant& negxyz = tree->FindNode(0b111, 1);
    ASSERT_EQ(negxyz.GetMaxDepth(), 2);
    ASSERT_EQ(negxyz.partition_location, 0b111);

    auto idx = kLocationIndex.at(0b111);
    ASSERT_EQ(negxyz.partition_index, idx);

    ASSERT_EQ(negxyz.Min(), glm::vec3(-0.5f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(-0.5f), negxyz.Min());
    ASSERT_EQ(negxyz.Max(), glm::vec3(0.f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(-0.5f), negxyz.Min());
  }

  {
    Octant& negxposy = tree->FindNode(0b001, 1);
    ASSERT_EQ(negxposy.GetMaxDepth(), 2);
    ASSERT_EQ(negxposy.partition_location, 0b001);

    auto idx = kLocationIndex.at(0b001);
    ASSERT_EQ(negxposy.partition_index, idx);
    ASSERT_EQ(negxposy.Min(), glm::vec3(0.0f, 0.0f, -0.5f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.0f, 0.0f, -0.5f), negxposy.Min());
    ASSERT_EQ(negxposy.Max(), glm::vec3(0.5f, 0.5f, 0.0f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.0f, 0.0f, -0.5f), negxposy.Min());
  }

  {
    Octant& posxnegy = tree->FindNode(0b110, 1);
    ASSERT_EQ(posxnegy.GetMaxDepth(), 2);
    ASSERT_EQ(posxnegy.partition_location, 0b110);

    auto idx = kLocationIndex.at(0b110);
    ASSERT_EQ(posxnegy.partition_index, idx);
    ASSERT_EQ(posxnegy.Min(), glm::vec3(-0.5f, -0.5f, 0.0f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(-0.5f, -0.5f, 0.0f), posxnegy.Min());
    ASSERT_EQ(posxnegy.Max(), glm::vec3(0.0f, 0.0f, 0.5f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(-0.5f, -0.5f, 0.0f), posxnegy.Min());
  }
}

TEST_F(OctreeTests, search_for_point) {
  {
    Ref<Octree> tree = NewRef<Octree>(glm::vec3{0.f, 0.f, 0.f});
    ASSERT_NE(tree, nullptr);

    tree->Subdivide({100.f, 100.f, 100.f}, 4);

    /// the only space containing the origin is the root
    Octant& octant = tree->GetContainingNode(glm::vec3{0.f, 0.f, 0.f});
    ASSERT_EQ(octant.GetMaxDepth(), 4);
    ASSERT_EQ(octant.tree_index, 0);
    ASSERT_EQ(octant.partition_index, 0);
    ASSERT_EQ(octant.partition_location, 0b000);
    ASSERT_EQ(octant.Min(), glm::vec3(-50.f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(-50.f), octant.Min());
    ASSERT_EQ(octant.Max(), glm::vec3(50.f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(50.f), octant.Max());
  }

  {  /// does not contain boundaries
    Ref<Octree> tree = NewRef<Octree>(glm::vec3{0.f, 0.f, 0.f});
    ASSERT_NE(tree, nullptr);

    tree->Subdivide({100.f, 100.f, 100.f}, 0);

    Octant& octant = tree->GetContainingNode(glm::vec3{50.f, 50.f, 50.f});
    ASSERT_EQ(octant.GetMaxDepth(), 0);
    ASSERT_EQ(octant.tree_index, 0);
    ASSERT_EQ(octant.partition_index, 0);
    ASSERT_EQ(octant.partition_location, 0b000);
    ASSERT_EQ(octant.Min(), glm::vec3(-50.0f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(-50.0f), octant.Min());
    ASSERT_EQ(octant.Max(), glm::vec3(50.f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(50.f), octant.Max());
  }

  {
    Ref<Octree> tree = NewRef<Octree>(glm::vec3{0.f, 0.f, 0.f});
    ASSERT_NE(tree, nullptr);

    tree->Subdivide({100.f, 100.f, 100.f}, 1);

    Octant& octant = tree->GetContainingNode(glm::vec3{25.f, 25.f, 25.f});
    ASSERT_EQ(octant.GetMaxDepth(), 0);
    ASSERT_EQ(octant.partition_index, 0);
    ASSERT_EQ(octant.partition_location, 0b000);
    ASSERT_EQ(octant.Min(), glm::vec3(0.f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(0.f), octant.Min());
    ASSERT_EQ(octant.Max(), glm::vec3(50.f))
      << fmtstr("Expected : {} , Actual : {}", glm::vec3(50.f), octant.Max());
  }
}

TEST_F(OctreeTests, non_0_origin) {
  constexpr static uint32_t kDepth = 2;

  Ref<Octree> tree = NewRef<Octree>(glm::vec3{0.f, 0.f, 0.f});
  ASSERT_NE(tree, nullptr);
}

TEST_F(OctreeTests, death_tests) {
  Ref<Octree> tree = NewRef<Octree>(glm::vec3{0.f, 0.f, 0.f});
  ASSERT_NE(tree, nullptr);

  ASSERT_NO_FATAL_FAILURE(tree->Subdivide({1.f, 1.f, 1.f}, 1));
  ASSERT_NE(tree->NumNodes(), 0);
  ASSERT_NE(tree->Depth(), 0);

  ASSERT_DEATH(tree->Rebuild(), "");
  /// ... others???
}

void OctreeTests::SetUpTestSuite() {
  ConfigTable test_config = ConfigTable{};
  test_config.Add("log", "console-level", "trace", true);
  test_config.Add("log", "file-level", "trace", true);
  test_config.Add("log", "path", "logs/octree-test.log", true);

  Logger::Open(test_config);
  Logger::Instance()->RegisterThread("Script Scene Integration Test Main Thread");
}

void OctreeTests::TearDownTestSuite() {
  CloseLog();
}
