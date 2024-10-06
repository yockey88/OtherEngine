/**
 * \file scene/octree.cpp
 **/
#include "scene/octree.hpp"

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
    
  static Octant null_octant;

} // namespace <anonymous>

  Octant::~Octant() {
    for (auto& c : children) {
      c = nullptr;
    }
  }
  
  Octree::Octree()
      : depth(1), num_octants(NumOctantsAtDepth(1)) {
    Initialize({ 1.f , 1.f , 1.f });
  }

  Octree::Octree(const glm::vec3& space_dimensions , size_t depth) 
      : depth(depth) , num_octants(NumOctantsAtDepth(depth)) {
    Initialize(space_dimensions);
  }

  Octree::~Octree() {
    octants.Clear();
  }
      
  const size_t Octree::Depth() const { 
    return kDepth; 
  }
  
  const size_t Octree::NumOctants() const { 
    return num_octants; 
  }
      
  const Octant& Octree::GetSpace() const { 
    return octants[space_index]; 
  }
  
  const glm::vec3& Octree::Dimensions() const { 
    return GetSpace().dimensions; 
  }

  Octant& Octree::GetOctant(const glm::vec3& point) {
    return FindOctant(GetSpace() , point);
  }

  void Octree::PrintOctants(std::ostream& os) const {
    PrintOctant(os , GetSpace());
  }
  
  void Octree::PrintOctant(std::ostream& os , const Octant& octant) const {
    os << fmtstr("Octant [{}] = {:p}\n" , octant.tree_index , static_cast<const void*>(&octant));
    os << fmtstr("(@Depth = {} , @Partition = {} , @Location = {:>03b} , @Dimensions = ({}) , @Center = ({}))\n", 
                  octant.depth , octant.partition_index , octant.partition_location , octant.dimensions, octant.origin);
    
    std::stringstream ss;
    if (octant.entities.size() > 0) {
      ss << "\n";
    }
    for (const auto& e : octant.entities) {
      ss << fmtstr(" - {}\n" , e->Name()); 
    }

    os << fmtstr("(@Entities = {}{})\n" , octant.entities.size() , ss.str());
    os << fmtstr(" - Parent = {}\n", static_cast<void*>(octant.parent));
    for (size_t i = 0; i < kNumChildren; ++i) {
      os << fmtstr(" -- [{}] = {} [{}]\n", i , octant.tree_index , static_cast<void*>(octant.children[i]));
    }

    os << "---|";
    for (const auto& c : octant.children) {
      PrintOctant(os , *c);
    }
    os << "---|";
  }
  
  void Octree::AddEntity(Entity* entity) {
    OE_ASSERT(entity != nullptr , "Attempting to add null entity to octree");

    // const Transform transform = entity->GetComponent<Transform>();
    // const glm::vec3 position = transform.position;

    // Octant& octant = FindOctant(octants[0] , position);
    // if (octant.tree_index > 0) {
    //   octant.entities.push_back(entity);
    //   return;
    // } 

    // OE_ERROR("Failed to add {} to octree at <{},{},{}>" , entity->Name() , 
    //           position.x , position.y , position.z);
  }

  Octant& Octree::GetSpace() {
    return octants[space_index];
  }
  
  void Octree::Initialize(const glm::vec3& dim) {
    dimensions = dim;
    
    {
      auto [idx , space] = octants.EmplaceBackNoLock();
      // o.tree_index = idx;
      // space_index = idx;
    }
    // space_index = idx;
    // space.tree_index = idx;
    // auto [idx , space] = octants.EmplaceBackNoLock();
    // Subdivide(space , depth);
  } 

  bool Octree::OctantContainsPoint(const Octant& octant , const glm::vec3& point) const {
    if (octant.origin == point) {
      return true;
    }

    glm::vec3 min = octant.origin - (octant.dimensions / 2.f);
    glm::vec3 max = octant.origin + (octant.dimensions / 2.f);

    bool greater_than_min = point.x > min.x && point.y > min.y && point.z > min.z;
    bool less_than_max = point.x < max.x && point.y < max.y && point.z < max.z;

    return greater_than_min && less_than_max;
  }

  Octant& Octree::FindOctant(Octant& octant , const glm::vec3& point) {
    /// origin is not contained in any children so this will be the smallest octant that contains the point
    if (point == octant.origin) {
      return octant;
    }

    for (const auto& o : octant.children) {
      Octant& oct = FindOctant(*o , point);
      if (oct.tree_index != -1) {
        return oct;
      }
    }

    return null_octant;
  }

  void Octree::Subdivide(Octant& octant, size_t depth) {
    octant.depth = depth;
    if (depth == 0) {
      return;
    }

    Subdivide(octant);
    for (auto& child : octant.children) {
      Subdivide(*child, depth - 1);
    }
  }

  void Octree::Subdivide(Octant& octant) {
    for (size_t i = 0; i < kNumChildren; ++i) {
      auto [idx , child] = octants.EmplaceBackNoLock();

      child.parent = &octant;
      child.tree_index = idx;

      child.partition_index = i;
      child.partition_location = kOctantLocations[i];

      child.dimensions = CalculateOctantDimensions(octant.dimensions);
      child.origin = CalculateOctantOrigin(octant.dimensions , octant.origin , kOctantLocations[i]);
      child.depth = octant.depth + 1;
      
      octant.children[i] = &child;
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

} // namespace other
