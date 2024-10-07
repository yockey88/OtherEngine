/**
 * \file unit_test/octree_tests.cpp
 **/
#include "core/config.hpp"
#include "oetest.hpp"

#include <glm/glm.hpp>

#include "core/logger.hpp"
#include "scene/octree.hpp"

class OctreeTests : public other::OtherTest {
  public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

using other::fmtstr;

using other::Ref;
using other::NewRef;

using other::ConfigTable;
using other::Logger;


using other::Octree;
using other::Octant;

using other::kLocationIndex;
using other::kOctantLocations;

TEST_F(OctreeTests , depth_0) {
  constexpr static uint32_t kDepth = 0;

  Ref<Octree> tree = NewRef<Octree>(glm::vec3{ 1.f } , kDepth);
  ASSERT_NE(tree , nullptr);
  ASSERT_EQ(tree->NumOctants() , 1);
  ASSERT_EQ(tree->Depth() , kDepth);

  Octant& space = tree->GetSpace();
  ASSERT_TRUE(space.IsLeaf());
  ASSERT_EQ(space.depth , 0);
  ASSERT_EQ(space.tree_index , 0);
  ASSERT_EQ(space.partition_index , 0);
  ASSERT_EQ(space.partition_location , 0b000);
  ASSERT_EQ(space.Min() , glm::vec3(-0.5f))
    << fmtstr("Expected : {} , Actual : {}" , glm::vec3(-0.5f) , space.Min());
  ASSERT_EQ(space.Max() , glm::vec3(0.5f))
    << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.5f) , space.Max());
}

TEST_F(OctreeTests , depth_1) {
  constexpr static uint32_t kDepth = 1;

  Ref<Octree> tree = NewRef<Octree>(glm::vec3{ 1.f } , kDepth);
  ASSERT_NE(tree , nullptr);
  ASSERT_EQ(tree->NumOctants() , 1 + 8);
  ASSERT_EQ(tree->Depth() , kDepth);

  Octant& space = tree->GetSpace();
  ASSERT_FALSE(space.IsLeaf());
  ASSERT_EQ(space.depth , 1);
  ASSERT_EQ(space.tree_index , 0);
  ASSERT_EQ(space.partition_index , 0);
  ASSERT_EQ(space.partition_location , 0b000);
  
  for (auto& c : space.Children()) {
    ASSERT_NE(c , nullptr);
    ASSERT_TRUE(c->IsLeaf());
    ASSERT_EQ(c->depth , 0);

    auto&idx = kLocationIndex.at(c->partition_location);
    ASSERT_EQ(c->partition_index , idx);

    if (c->partition_location == 0b000) {
      ASSERT_EQ(c->Min() , glm::vec3(0.f))
        << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.0f) , c->Min());
      ASSERT_EQ(c->Max() , glm::vec3(0.5f))
        << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.5f) , c->Max());
    } else if (c->partition_location == 0b111) {
      ASSERT_EQ(c->Min() , glm::vec3(-0.5f))
        << fmtstr("Expected : {} , Actual : {}" , glm::vec3(-0.5f) , c->Min());
      ASSERT_EQ(c->Max() , glm::vec3(0.f))
        << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.0f) , c->Max());
    }
  }
}

TEST_F(OctreeTests , higher_res_2) {
  constexpr static uint32_t kDepth = 2;

  Ref<Octree> tree = NewRef<Octree>(glm::vec3{ 1.f } , kDepth);
  ASSERT_NE(tree , nullptr);

  {
    Octant& space = tree->GetSpace();
    ASSERT_FALSE(space.IsLeaf());
    ASSERT_EQ(space.depth , 2);
    ASSERT_EQ(space.tree_index , 0);
    ASSERT_EQ(space.partition_index , 0);
    ASSERT_EQ(space.partition_location , 0b000);

    for (auto& c : space.Children()) {
      ASSERT_NE(c , nullptr);
      ASSERT_FALSE(c->IsLeaf());
      ASSERT_EQ(c->depth , 1);

      auto&idx = kLocationIndex.at(c->partition_location);
      ASSERT_EQ(c->partition_index , idx);
      if (c->partition_location == 0b000) {
        ASSERT_EQ(c->Min() , glm::vec3(0.f))
          << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.0f) , c->Min());
        ASSERT_EQ(c->Max() , glm::vec3(0.5f))
          << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.5f) , c->Max());
      }

      for (auto& gc : c->Children()) {
        ASSERT_NE(gc , nullptr);
        ASSERT_TRUE(gc->IsLeaf());
        ASSERT_EQ(gc->depth , 0);

        auto idx = kLocationIndex.at(gc->partition_location);
        ASSERT_EQ(gc->partition_index , idx);
      }
    }
  }

  {
    Octant& posxyz = tree->FindOctant(0b000 , 1);
    ASSERT_EQ(posxyz.depth , 1);
    ASSERT_EQ(posxyz.partition_location , 0b000);
    ASSERT_EQ(posxyz.Min() , glm::vec3(0.0f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.0f) , posxyz.Min());
    ASSERT_EQ(posxyz.Max() , glm::vec3(0.5f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.5f) , posxyz.Max());

    auto idx = kLocationIndex.at(0b000);
    ASSERT_EQ(posxyz.partition_index ,  idx);

  }

  {
    Octant& negxyz = tree->FindOctant(0b111 , 1);
    ASSERT_EQ(negxyz.depth , 1);
    ASSERT_EQ(negxyz.partition_location , 0b111);
    ASSERT_EQ(negxyz.Min() , glm::vec3(-0.5f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(-0.5f) , negxyz.Min());
    ASSERT_EQ(negxyz.Max() , glm::vec3(0.f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.f) , negxyz.Max());

    auto idx = kLocationIndex.at(0b111);
    ASSERT_EQ(negxyz.partition_index , idx);
  }

  {
    Octant& negxposy = tree->FindOctant(0b001 , 1);
    ASSERT_EQ(negxposy.depth , 1);
    ASSERT_EQ(negxposy.partition_location , 0b001);
    ASSERT_EQ(negxposy.Min() , glm::vec3(0.0f , 0.0f , -0.5f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.0f , 0.0f , -0.5f) , negxposy.Min());
    ASSERT_EQ(negxposy.Max() , glm::vec3(0.5f , 0.5f ,  0.0f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.5f , 0.5f ,  0.0f) , negxposy.Max());

    auto idx = kLocationIndex.at(0b001);
    ASSERT_EQ(negxposy.partition_index , idx);
  }

  {
    Octant& posxnegy = tree->FindOctant(0b110 , 1);
    ASSERT_EQ(posxnegy.depth , 1);
    ASSERT_EQ(posxnegy.partition_location , 0b110);
    ASSERT_EQ(posxnegy.Min() , glm::vec3(-0.5f , -0.5f , 0.0f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(-0.5f , -0.5f , 0.0f) , posxnegy.Min());
    ASSERT_EQ(posxnegy.Max() , glm::vec3( 0.0f ,  0.0f , 0.5f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3( 0.0f ,  0.0f , 0.5f) , posxnegy.Max());

    auto idx = kLocationIndex.at(0b110);
    ASSERT_EQ(posxnegy.partition_index , idx);
  }

  auto& space = tree->GetSpace();
  ASSERT_EQ(space.Min() , space.FindFurthestOctant(0b111).Min());
  ASSERT_EQ(space.Max() , space.FindFurthestOctant(0b000).Max());
}

TEST_F(OctreeTests , higher_res_3) {
  constexpr static uint32_t kDepth = 3;

  Ref<Octree> tree = NewRef<Octree>(glm::vec3{ 1.f , 1.f , 1.f } , kDepth);
  ASSERT_NE(tree , nullptr);

  {
    Octant& space = tree->GetSpace();
    ASSERT_EQ(space.depth , 3);
    ASSERT_EQ(space.tree_index , 0);
    ASSERT_EQ(space.partition_index , 0);
    ASSERT_EQ(space.partition_location , 0b000);

    for (auto& c : space.Children()) {
      ASSERT_NE(c , nullptr);
      ASSERT_FALSE(c->IsLeaf());
      ASSERT_EQ(c->depth , 2);

      for (auto& gc : c->Children()) {
        ASSERT_NE(gc , nullptr);
        ASSERT_FALSE(gc->IsLeaf());
        ASSERT_EQ(gc->depth , 1);

        for (auto& ggc : gc->Children()) {
          ASSERT_NE(ggc , nullptr);
          ASSERT_TRUE(ggc->IsLeaf());
          ASSERT_EQ(ggc->depth , 0);
        }
      }
    }
  }

  {
    Octant& posxyz = tree->FindOctant(0b000 , 1);
    ASSERT_EQ(posxyz.depth , 2);
    ASSERT_EQ(posxyz.partition_location , 0b000);

    auto idx = kLocationIndex.at(0b000);
    ASSERT_EQ(posxyz.tree_index , 1 + idx);
    ASSERT_EQ(posxyz.partition_index ,  idx);
    ASSERT_EQ(posxyz.Min() , glm::vec3(0.0f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.0f) , posxyz.Min());
    ASSERT_EQ(posxyz.Max() , glm::vec3(0.5f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.5f) , posxyz.Max());
  }

  {
    Octant& negxyz = tree->FindOctant(0b111 , 1);
    ASSERT_EQ(negxyz.depth , 2);
    ASSERT_EQ(negxyz.partition_location , 0b111);

    auto idx = kLocationIndex.at(0b111);
    ASSERT_EQ(negxyz.tree_index , 1 + idx);
    ASSERT_EQ(negxyz.partition_index , idx);
    ASSERT_EQ(negxyz.Min() , glm::vec3(-0.5f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(-0.5f) , negxyz.Min());
    ASSERT_EQ(negxyz.Max() , glm::vec3(0.f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(-0.5f) , negxyz.Min());
  }

  {
    Octant& negxposy = tree->FindOctant(0b001 , 1);
    ASSERT_EQ(negxposy.depth , 2);
    ASSERT_EQ(negxposy.partition_location , 0b001);

    auto idx = kLocationIndex.at(0b001);
    ASSERT_EQ(negxposy.tree_index , 1 + idx);
    ASSERT_EQ(negxposy.partition_index , idx);
    ASSERT_EQ(negxposy.Min() , glm::vec3(0.0f , 0.0f , -0.5f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.0f , 0.0f , -0.5f) , negxposy.Min());
    ASSERT_EQ(negxposy.Max() , glm::vec3(0.5f , 0.5f ,  0.0f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(0.0f , 0.0f , -0.5f) , negxposy.Min());
  }

  {
    Octant& posxnegy = tree->FindOctant(0b110 , 1);
    ASSERT_EQ(posxnegy.depth , 2);
    ASSERT_EQ(posxnegy.partition_location , 0b110);

    auto idx = kLocationIndex.at(0b110);
    ASSERT_EQ(posxnegy.tree_index , 1 + idx);
    ASSERT_EQ(posxnegy.partition_index , idx);
    ASSERT_EQ(posxnegy.Min() , glm::vec3(-0.5f , -0.5f , 0.0f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(-0.5f , -0.5f , 0.0f) , posxnegy.Min());
    ASSERT_EQ(posxnegy.Max() , glm::vec3( 0.0f ,  0.0f , 0.5f))
      << fmtstr("Expected : {} , Actual : {}" , glm::vec3(-0.5f , -0.5f , 0.0f) , posxnegy.Min());
  }
}

void OctreeTests::SetUpTestSuite() {
  ConfigTable test_config = ConfigTable{};
  test_config.Add("log", "console-level" , "trace" , true);
  test_config.Add("log", "file-level" , "trace" , true);
  test_config.Add("log", "path" , "logs/octree-test.log" , true);
  
  Logger::Open(test_config);
  Logger::Instance()->RegisterThread("Script Scene Integration Test Main Thread");
}

void OctreeTests::TearDownTestSuite() {
  CloseLog();
}

