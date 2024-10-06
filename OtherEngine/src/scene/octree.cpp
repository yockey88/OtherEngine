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

} // namespace <anonymous>

  /// TODO: replace println with log
  
  Octree::Octree()
      : depth(1) , num_octants(NumOctantsAtDepth(1)) {
    Initialize({ 1.f , 1.f , 1.f });
  }

  Octree::Octree(const glm::vec3& space_dimensions , size_t depth) 
      : depth(depth) , num_octants(NumOctantsAtDepth(depth)) {
    Initialize(space_dimensions);
  }

  Octree::~Octree() {
    space = nullptr;
  }
      
  const size_t Octree::Depth() const { 
    return kDepth; 
  }
  
  const size_t Octree::NumOctants() const { 
    return num_octants; 
  }
      
  Scope<Octant>& Octree::GetSpace() { 
    return space; 
  }
  
  const glm::vec3& Octree::Dimensions() const { 
    return space->dimensions; 
  }

  static Scope<Octant> null_oct = NewScope<Octant>();
  const Scope<Octant>& Octree::GetOctant(const glm::vec3& point) const {
    OE_ASSERT(space != nullptr , "Cant retrieve octant from null space!");
    return FindOctant(space , point);
  }

  void Octree::PrintOctants(std::ostream& os) const {
    PrintOctant(os , space);
  }
  
  void Octree::PrintOctant(std::ostream& os , const Scope<Octant>& octant) const {
    OE_ASSERT(octant != nullptr , "Cant print null octant!");

    os << fmtstr("Octant [{}] = {:p}\n" , octant->tree_index , static_cast<const void*>(octant.get()));
    os << fmtstr("(@Depth = {} , @Partition = {} , @Location = {:>03b} , @Dimensions = ({}) , @Center = ({}))\n", 
                  octant->depth , octant->partition_index , octant->partition_location , octant->dimensions, octant->origin);
    
    std::stringstream ss;
    if (octant->entities.size() > 0) {
      ss << "\n";
    }
    for (const auto& e : octant->entities) {
      ss << fmtstr(" - {}\n" , e->Name()); 
    }

    os << fmtstr("(@Entities = {}{})\n" , octant->entities.size() , ss.str());
    os << fmtstr(" - Parent = {}\n", static_cast<void*>(octant->parent));
    for (size_t i = 0; i < kNumChildren; ++i) {
      os << fmtstr(" -- [{}] = {} [{}]\n", i , octant->tree_index , static_cast<void*>(octant->children[i].get()));
    }

    os << "---|";
    for (const auto& c : octant->children) {
      PrintOctant(os , c);
    }
    os << "---|";
  }

  bool Octree::OctantContainsPoint(const Scope<Octant>& octant , const glm::vec3& point) const {
    OE_ASSERT(octant != nullptr , "Attempting to check null octant for point!");

    if (octant->origin == point) {
      return true;
    }

    glm::vec3 min = octant->origin - (octant->dimensions / 2.f);
    glm::vec3 max = octant->origin + (octant->dimensions / 2.f);

    bool greater_than_min = point.x > min.x && point.y > min.y && point.z > min.z;
    bool less_than_max = point.x < max.x && point.y < max.y && point.z < max.z;

    return greater_than_min && less_than_max;
  }

  const Scope<Octant>& Octree::FindOctant(const Scope<Octant>& octant , const glm::vec3& point) const {
    /// should be impossible, we should always find the point before recursing to leaf node's null 'children'
    OE_ASSERT(octant != nullptr , "Can not search for point will null root octant");
    
    /// origin is not contained in any children so this will be the smallest octant that contains the point
    if (point == octant->origin) {
      return octant;
    }

    for (const auto& o : octant->children) {
      const Scope<Octant>& oct = FindOctant(o , point);
      if (oct->tree_index != -1) {
        return oct;
      }
    }

    OE_ERROR("Point is outside of space!");
    static Scope<Octant> null_octant = NewScope<Octant>();
    null_octant->tree_index = -1;
    return null_octant;
  }

  void Octree::Subdivide(Scope<Octant>& octant, size_t depth) {
    octant->depth = depth;
    if (depth == 0) {
      return;
    }

    Subdivide(octant);
    for (auto& child : octant->children) {
      Subdivide(child, depth - 1);
    }
  }

  void Octree::Subdivide(Scope<Octant>& octant) {
    OE_ASSERT(octant != nullptr , "Cant subdivide null octant!");
    for (size_t i = 0; i < kNumChildren; ++i) {
      octant->children[i] = NewScope<Octant>(octant); 
      octant->children[i]->tree_index = idx_generator++;

      octant->children[i]->partition_index = i;
      octant->children[i]->partition_location = kOctantLocations[i];

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
    OE_ASSERT(entity != nullptr , "Attempting to add null entity to octree");
    OE_ASSERT(space != nullptr , "Space is null!");

    const Transform transform = entity->GetComponent<Transform>();
    const glm::vec3 position = transform.position;

    const Scope<Octant>& octant = FindOctant(space , position);
    if (octant->tree_index > 0) {
      octant->entities.push_back(entity);
      return;
    } 

    OE_ERROR("Failed to add {} to octree at <{},{},{}>" , entity->Name() , 
              position.x , position.y , position.z);
  }
  
  void Octree::Initialize(const glm::vec3& dim) {
    dimensions = dim;

    space = NewScope<Octant>(); 
    space->dimensions = dim;
    Subdivide(space , depth);
  } 

} // namespace other
