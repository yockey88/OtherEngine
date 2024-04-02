/**
 * \file octree-tests/tests2.cpp
 **/
#include "scene/octree.hpp"

#include <gtest/gtest.h>

#include "octree_tests_helpers.hpp"

class OctreeTests2 : public ::testing::Test {
  public:
    OctreeTests2() {}
    virtual ~OctreeTests2() override {}
};

TEST_F(OctreeTests2 , coordinates1) {
  float dim = 3.f;
  size_t depth = 1;
  
  other::Octree octree(glm::vec3{ dim } , depth); // defaults to kDepth = 3 
  
  other::Octant* octant = octree.GetOctant({ 0 , 0 , 0 }); // root
  ASSERT_NE(octant, nullptr)
    << "octant is null";

  EXPECT_EQ(octree.GetSpace(), octant);
  EXPECT_EQ(octree.NumOctants(), GetSeriesSum(other::kNumChildren , depth));

  octant = octree.GetOctant({ 1 , 1 , 1 });
  ASSERT_NE(octant, nullptr)
    << "octant is null";

  EXPECT_EQ(octant->depth, depth);
  EXPECT_EQ(octant->partition_index, other::kPxPyPz);
  EXPECT_EQ(octant->location, other::kPxPyPzLoc);
  EXPECT_EQ(octant->origin, glm::vec3{ 0.75f });
  EXPECT_EQ(octant->dimensions, glm::vec3{ dim / 2 });
  EXPECT_EQ(octant->parent, octree.GetSpace());
}

TEST_F(OctreeTests2 , coordinates2) {
  float dim = 10.f;
  size_t depth = 5;

  other::Octree octree(glm::vec3{ dim } , depth); // defaults to kDepth = 3
}
