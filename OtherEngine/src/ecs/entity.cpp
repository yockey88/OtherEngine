/**
 * \file ecs/entity.cpp
 **/
#include "ecs/entity.hpp"

#include "scene/scene.hpp"
#include "scene/octree.hpp"

namespace other {

  void Entity::PlaceInSpace(Octant* space) {
    if (space == nullptr) {
      return;
    }

    containing_space = space;
    containing_space->entities.push_back(this);
  }

} // namespace other
