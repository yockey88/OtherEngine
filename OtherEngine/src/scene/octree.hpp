/**
 * \file scene/octree.hpp
 **/
#ifndef OTHER_ENGINE_OCTREE_HPP
#define OTHER_ENGINE_OCTREE_HPP

#include <string_view>

#include <spdlog/fmt/fmt.h>

#include "scene/bvh.hpp"
#include "scene/bvh_node.hpp"

namespace other {

  using Octant = BvhNode<8>;
  using Octree = Bvh<8>;

}  // namespace other

#endif  // !OTHER_ENGINE_OCTREE_HPP
