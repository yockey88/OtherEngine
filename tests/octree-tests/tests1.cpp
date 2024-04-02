/**
 * \file octree-tests/tests1.cpp
 **/
#include "scene/octree.hpp"

#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>

#include "octree_tests_helpers.hpp"

class OctreeTests1 : public ::testing::Test {
  public:
    OctreeTests1() {}
    virtual ~OctreeTests1() override {}
};

TEST_F(OctreeTests1 , ctor_and_subdivide) {
  other::Octree octree({ 1 , 1 , 1 }); // defaults to kDepth = 3 
  
  other::Octant* space = octree.GetSpace();
  ASSERT_NE(space, nullptr);

  ASSERT_EQ(octree.NumOctants(), GetSeriesSum(8 , other::kDepth));
}

TEST_F(OctreeTests1 , ctor_and_subdivide_depth_1) {
  other::Octree octree({ 1 , 1 , 1 } , 1); 
  
  other::Octant* space = octree.GetSpace();
  ASSERT_NE(space, nullptr);

  ASSERT_EQ(octree.NumOctants(), GetSeriesSum(8 , 1));
}

TEST_F(OctreeTests1 , ctor_and_subdivide_depth_2) {
  other::Octree octree({ 1 , 1 , 1 } , 2); 
  
  other::Octant* space = octree.GetSpace();
  ASSERT_NE(space, nullptr);

  ASSERT_EQ(octree.NumOctants(), GetSeriesSum(8 , 2));
}

TEST_F(OctreeTests1 , get_node) {
  other::Octree octree({ 1 , 1 , 1 } , 1); 
  
  other::Octant* space = octree.GetSpace();
  ASSERT_NE(space, nullptr);

  other::Octant* octant = octree.GetOctant(0 , 0);
  ASSERT_NE(octant, nullptr);

  EXPECT_EQ(space , octant)
    << fmt::format(fmt::runtime("space != octant ({:p} != {:p})"), (void*)space, (void*)octant);

  for (size_t i = 0; i < other::kNumChildren; ++i) {
    octant = octree.GetOctant(1 , i);
    ASSERT_NE(octant, nullptr);

    EXPECT_EQ(octant->depth , 1);
    // special case on second level, will test larger trees seperately
    EXPECT_EQ(octant->tree_index , i + 1);
    EXPECT_EQ(octant->partition_index , i);
    EXPECT_EQ(octant->parent , space);
  }
}

TEST_F(OctreeTests1 , negative_tests) {
  other::Octree octree({ 1 , 1 , 1 } , 1);

  /// should fail because partition index is out of bounds
  EXPECT_EQ(octree.GetOctant(1 , 8) , nullptr);
  /// should fail because depth is out of bounds
  EXPECT_EQ(octree.GetOctant(2 , 0) , nullptr);
  /// should fail because depth is out of bounds
  /// and partition index is out of bounds
  EXPECT_EQ(octree.GetOctant(2 , 8) , nullptr);
}

#if 0
TEST_F(OctreeTests1 , print_octants) {
  other::Octree octree({ 3 , 3 , 3 } , 3);
  octree.PrintOctants();
}
#endif
