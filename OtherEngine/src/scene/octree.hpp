/**
 * \file scene/octree.hpp
 **/
#ifndef OTHER_ENGINE_OCTREE_HPP
#define OTHER_ENGINE_OCTREE_HPP

#include <string_view>

#include <spdlog/fmt/fmt.h>

#include "scene/bvh_node.hpp"
#include "scene/bvh.hpp"

namespace other {

  using Octant = BvhNode<8 , OCTREE>;
  using Octree = Bvh<8 , OCTREE>;

} // namespace other

#endif // !OTHER_ENGINE_OCTREE_HPP
