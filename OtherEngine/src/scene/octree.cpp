/**
 * \file scene/octree.cpp
 **/
#include "scene/octree.hpp"

#include <ranges>

#include "core/logger.hpp"
#include "ecs/entity.hpp"

namespace other {
namespace {

  constexpr static size_t FastPow(size_t base , size_t exp) {
    size_t result = 1;
    while (exp > 0) {
      if (exp & 1) {
        result *= base;
      }
      base *= base;
      exp >>= 1;
    }
    return result;
  }

  /// num octants at depth n = 1 + 8 + 64 + ... = 8^0 + 8^1 + 8^2 + ... + 8^n
  constexpr static size_t NumOctantsAtDepth(size_t depth) {
    size_t sum = 0;
    for (size_t i = 0; i <= depth; ++i) {
      sum += FastPow(kNumChildren, i);
    }
    return sum;
  }

  constexpr static uint8_t LocationFromPoint(const glm::vec3& point , const glm::vec3& center , const glm::vec3& dimensions) {
    uint8_t location = 0;
    if (point.x < center.x - dimensions.x / 2.f) {
      location |= kNegativeXBit;
    }
    if (point.y < center.y - dimensions.y / 2.f) {
      location |= kNegativeYBit;
    }
    if (point.z < center.z - dimensions.z / 2.f) {
      location |= kNegativeZBit;
    }
    return location;
  }

} // namespace <anonymous>

  /// TODO: replace println with log

  Octree::Octree(const glm::vec3& space_dimensions , size_t depth) 
      : depth(depth) , num_octants(NumOctantsAtDepth(depth)) {
    octants = std::vector<Octant>(num_octants);
    space = CurrentOctant();
    space->dimensions = space_dimensions;
    Subdivide(space , depth);
  }

  Octree::~Octree() {
    // reset the space
    octants = {};
    space = nullptr;
  }

  Octant* Octree::GetOctant(size_t index) {
    if (index >= octants.size()) {
      println("Index out of bounds");
      return nullptr;
    }

    return &octants[index];
  }

  Octant* Octree::GetOctant(const glm::vec3& point) const {
    if (point.x < (-1 * space->dimensions.x / 2.f) || point.x > space->dimensions.x / 2.f ||
        point.y < (-1 * space->dimensions.x / 2.f) || point.y > space->dimensions.y / 2.f ||
        point.z < (-1 * space->dimensions.x / 2.f) || point.z > space->dimensions.z / 2.f) {
      println("Point out of bounds");
      return nullptr;
    }

    return FindOctant(space , point);
  }
  
  Octant* Octree::GetOctant(size_t d, size_t index) {
    if (d == 0) {
      if (index != 0) {
        println("0-Depth layer of tree has only one octant, the space itself");
      }

      return space;
    } else if (d > depth) {
      println("Depth out of bounds");
      return nullptr;
    }

    if (index >= kNumChildren) {
      println("Index out of bounds");
      return nullptr;
    }

    /// If depth is 0 then there is only one octant, the root or the space
    if (d == 1) {
      return space->children[index];
    }

    size_t idx = (NumOctantsAtDepth(d - 1) + index) - 1;
    if (idx >= octants.size()) {
      println("Index out of bounds");
      return nullptr;
    }

    println("Returning octant at index = {}" , idx);

    return &octants[idx];
  }

  void Octree::PrintOctants() const {
    for (size_t i = 0; i < cursor; ++i) {
      PrintOctant(&octants[i]);
    }
  }
  
  void Octree::PrintOctant(const Octant* octant) const {
    println("Octant [{}] = {:p}" , octant->tree_index , static_cast<const void*>(octant));
    println("(@Depth = {} , @Partition = {} , @Location = {:>03b} , @Dimensions = ({},{},{}) , @Center = ({} , {} , {}))", 
        octant->depth , octant->partition_index , octant->location , 
        octant->dimensions.x , octant->dimensions.y , octant->dimensions.z ,
        octant->origin.x , octant->origin.y , octant->origin.z);
    println(" - Parent = {}", static_cast<void*>(octant->parent));
    for (size_t i = 0; i < kNumChildren; ++i) {
      println(" -- [{}] = {}", i , static_cast<void*>(octant->children[i]));
    }
  }

  bool Octree::OctantContainsPoint(const Octant* octant , const glm::vec3& point) {
    return point.x > octant->origin.x - (octant->dimensions.x / 2.f) && point.x < octant->origin.x + (octant->dimensions.x / 2.f) &&
           point.y > octant->origin.y - (octant->dimensions.y / 2.f) && point.y < octant->origin.y + (octant->dimensions.y / 2.f) &&
           point.z > octant->origin.z - (octant->dimensions.z / 2.f) && point.z < octant->origin.z + (octant->dimensions.z / 2.f);
  }

  Octant* Octree::FindOctant(Octant* octant , const glm::vec3& point) const {
    /// should be impossible, we should always find the point before recursing to leaf node's null 'children'
    if (octant == nullptr) {
      println("UNREACHABLE | DEV ERROR: Octant is null");
      return nullptr;
    }

    /// this should also be impossible because we should only ever pass octants that contain the point into this function
    if ((point.x < octant->origin.x - (octant->dimensions.x / 2.f) || point.x > octant->origin.x + (octant->dimensions.x / 2.f)) ||
        (point.y < octant->origin.y - (octant->dimensions.y / 2.f) || point.y > octant->origin.y + (octant->dimensions.y / 2.f)) ||
        (point.z < octant->origin.z - (octant->dimensions.y / 2.f) || point.z > octant->origin.z + (octant->dimensions.z / 2.f))) {
      println("UNREACHABLE | DEV ERROR: Point is not contained in octant");
      return nullptr;
    }

    /// origin is not contained in any children so this will be the smallest octant that contains the point
    if (point == octant->origin) {
      return octant;
    }

    /// octants are open (do not include their boundaries) so we need to check if the point is on the boundary
    /// the only exception is the space itself which is closed (includes its boundaries)
    if ((point.x == octant->origin.x - (octant->dimensions.x / 2.f) || point.x == octant->origin.x + (octant->dimensions.x / 2.f)) &&
        (point.y == octant->origin.y - (octant->dimensions.y / 2.f) || point.y == octant->origin.y + (octant->dimensions.y / 2.f)) &&
        (point.z == octant->origin.z - (octant->dimensions.z / 2.f) || point.z == octant->origin.z + (octant->dimensions.z / 2.f))) {
      return octant;
    }

    /// point is in bounds, is not on the boundary, and is not the origin so we need to recurse
    const size_t idx = kLocationIndex.at(LocationFromPoint(point , octant->origin , octant->dimensions));

    if (octant->children[idx] == nullptr) {
      /// this is the smallest octant that contains the point
      return octant;
    }

    return FindOctant(octant->children[idx] , point);
  }

  void Octree::Subdivide(Octant* octant, size_t depth) {
    if (depth == 0) {
      return;
    }

    Subdivide(octant);
    for (auto& child : octant->children) {
      Subdivide(child, depth - 1);
    }
  }

  void Octree::Subdivide(Octant* octant) {
    if (octant == nullptr) {
      return;
    }

    if (cursor >= octants.size()) {
      println("Cursor out of bounds");
      return;
    }

    if (cursor + kNumChildren > octants.size()) {
      println("Cursor + kNumChildren out of bounds , cursor = {}" , cursor);
      return;
    }

    for (size_t i = 0; i < kNumChildren; ++i) {
      octant->children[i] = CurrentOctant();
      /// cursor - 1 because we just incremented it in CurrentOctant
      octant->children[i]->tree_index = cursor - 1;
      octant->children[i]->partition_index = i;
      octant->children[i]->location = kOctantLocations[i];

      octant->children[i]->parent = octant;

      octant->children[i]->dimensions = CalculateOctantDimensions(octant->dimensions);
      octant->children[i]->origin = CalculateOctantOrigin(octant->dimensions , octant->origin , kOctantLocations[i]);

      octant->children[i]->depth = octant->depth + 1;
    }
  }

  glm::vec3 Octree::CalculateOctantOrigin(const glm::vec3& parent_dimensions , const glm::vec3& parent_origin , uint8_t location) {
    /// divide by 4 because origin is at (0,0,0) so we need shift in the direction half of half of the parent dimensions
    float x_shift = (parent_dimensions.x / 4.f) * (location & kNegativeXBit ? -1 : 1); 
    float y_shift = (parent_dimensions.y / 4.f) * (location & kNegativeYBit ? -1 : 1);
    float z_shift = (parent_dimensions.z / 4.f) * (location & kNegativeZBit ? -1 : 1);

    return {
      parent_origin.x + x_shift,
      parent_origin.y + y_shift,
      parent_origin.z + z_shift
    };
  } 

  glm::vec3 Octree::CalculateOctantDimensions(const glm::vec3& parent_dimensions) {
    return {
      parent_dimensions.x / 2.f,
      parent_dimensions.y / 2.f,
      parent_dimensions.z / 2.f
    };
  }
  
  void Octree::AddEntity(Entity* entity) {
    if (entity == nullptr) {
      return;
    }

    Octant* octant = GetOctant(entity->Position());
    Octant* previous_spot = entity->ContainingSpace();

    if (octant == previous_spot) {
      return;
    }

    /// remove entity from previous spot
    std::erase(previous_spot->entities , entity);

    /// add entity to new spot
    entity->PlaceInSpace(octant);
  }

} // namespace other
