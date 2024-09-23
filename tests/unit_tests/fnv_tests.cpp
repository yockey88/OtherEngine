/**
 * \file fnv_tests.cpp
 **/
#include "oetest.hpp"

#include "core/defines.hpp"

TEST(FnvTests , collision_test) {
  EXPECT_NE(other::FNV("Geometry") , other::FNV("Debug")); 
  EXPECT_NE(other::FNV("Geometry") , other::FNV("GEOMETRY"));
}

TEST(FnvTests , hash) {
  std::cout << other::FNV("sun") << std::endl;  
}
