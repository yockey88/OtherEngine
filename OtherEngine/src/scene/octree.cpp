/**
 * \file scene/octree.cpp
 **/
#include "scene/octree.hpp"

#include <algorithm>
#include <ranges>

#include "core/logger.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/transform.hpp"

#include "scene/scene.hpp"

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
  /** Octant Num Table:
   *  0 -> 1 = 8^0
   *  1 -> 9 = 8^0 + 8^1
   *  2 -> 73 = 8^0 + 8^1 + 8^2
   *  3 -> 585 = 8^0 + 8^1 + 8^2 + 8^3
   *  4 -> 4681 = 8^0 + 8^1 + 8^2 + 8^3 + 8^4
   *  ....
   **/
  constexpr static size_t NumOctantsAtDepth(size_t depth) {
    size_t sum = 0;
    for (size_t i = 0; i <= depth; ++i) {
      sum += FastPow(kNumChildren, i);
    }
    return sum;
  }
  
  /// treat point wrt to global coordinates
  constexpr static uint8_t LocationFromPoint(const glm::vec3& point) {
    uint8_t location = 0;
    if (point.x < 0.f) {
      location |= kNegativeXBit;
    }
    if (point.y < 0.f) {
      location |= kNegativeYBit;
    }
    if (point.z < 0.f) {
      location |= kNegativeZBit;
    }
    return location;
  }
  
  constexpr static uint8_t LocationFromPoint(const glm::vec3& point , const glm::vec3& center , const glm::vec3& dimensions) {
    /// translate to global origin
    glm::vec3 translated = point - (center - dimensions / 2.f);
    return LocationFromPoint(translated);
  }

} // anonymous namespace

  bool Octant::IsLeaf() const {
    return depth == 0;
  }

  const std::array<Octant* , kNumChildren>& Octant::Children() const { 
    return children; 
  }

  bool Octant::Contains(const glm::vec3& point) const { 
    return bbox.Contains(point); 
  }
   
  const glm::vec3& Octant::Min() const { 
    return bbox.min; 
  }

  const glm::vec3& Octant::Max() const { 
    return bbox.max; 
  }

  static Octant null_octant;
  Octant& Octant::FindOctant(const glm::vec3& point) {
    if (!Contains(point)) {
      return null_octant;
    }

    OE_TRACE(" > Searching for point {} in octant {}" , point , tree_index);
    if (IsLeaf()) {
      OE_TRACE(" > Found point {} in leaf octant {}" , point , tree_index);
      return *this;
    }

    for (const auto& octant : children) {
      if (octant->Contains(point)) {
        return *(octant->GetChild(point));
      }
    }

    OE_ASSERT(false , "Point not found in any child octant!");
  }
  
  Octant& Octant::FindOctant(uint8_t location , size_t at_depth) {
    return *GetOctant(location , at_depth);
  }

  Octant& Octant::FindFurthestOctant(uint8_t location) {
    return *GetFurthestOctant(location);
  }

  void Octant::ExpandToInclude(const glm::vec3& point) {
    OE_TRACE(" > Attempting to expand octant {} to include point {}" , tree_index , point);
    if (!IsLeaf()) {
      uint8_t loc = LocationFromPoint(point);
      auto* child = children[kLocationIndex.at(loc)];
      child->ExpandToInclude(point);
    } else {
      ExpandAround(point);
    }
  }

  void Octant::Subdivide(StableVector<Octant>& octants , size_t d) {
    depth = d;
    if (d == 0) {
      std::ranges::fill(children , nullptr);
      return;
    }

    Subdivide(octants);
    for (auto& child : children) {
      child->Subdivide(octants , depth - 1);
    }
  }
  
  Octant* Octant::GetChild(const glm::vec3& point) {
    if (IsLeaf() && bbox.Contains(point)) {
      return this;
    } else if (IsLeaf()) {
      return nullptr;
    }
    
    for (const auto& c : children) {
      if (c->bbox.Contains(point)) {
        OE_TRACE(" > Checking child {} for {}" , c->tree_index , point);
        return c->GetChild(point);
      }
    }

    OE_TRACE(" > Point {} not found in any child of octant {}" , point , tree_index);
    return nullptr;
  }
  
  Octant* Octant::GetOctant(uint8_t location , size_t at_depth) {
    if (depth == 0 || IsLeaf() || at_depth == 0) {
      return this;
    }

    auto idx = kLocationIndex.at(location);
    if (at_depth == 1) {
      return children[idx];
    }

    return children[idx]->GetOctant(location , at_depth - 1);
  }

  Octant* Octant::GetFurthestOctant(uint8_t location) {
    if (depth == 0 || IsLeaf()) {
      return this;
    }

    auto idx = kLocationIndex.at(location);
    return children[idx]->GetFurthestOctant(location);
  }
  
  glm::vec3 Octant::GetMinForSubQuadrant(uint8_t location) {
    switch (location) {
      case 0b000: return bbox.Center();
      case 0b111: return bbox.min;

      case 0b001:
        return glm::vec3(bbox.Center().x, bbox.Center().y, bbox.min.z);
      case 0b010:
        return glm::vec3(bbox.Center().x, bbox.min.y , bbox.Center().z);
      case 0b100:
        return glm::vec3(bbox.min.x , bbox.Center().y, bbox.Center().z);

      case 0b011:
        return glm::vec3(bbox.Center().x, bbox.min.y , bbox.min.z);
      case 0b101:
        return glm::vec3(bbox.min.x , bbox.Center().y, bbox.min.z);
      case 0b110:
        return glm::vec3(bbox.min.x , bbox.min.y , bbox.Center().z);

    
      default:
        OE_ASSERT(false , "Invalid location : {:>03b}" , location);
    }
  }

  glm::vec3 Octant::GetMaxForSubQuadrant(uint8_t location) {
    switch (location) {
      case 0b000: return bbox.max;
      case 0b111: return bbox.Center();

      case 0b001:
        return glm::vec3(bbox.max.x , bbox.max.y , bbox.Center().z);
      case 0b010:
        return glm::vec3(bbox.max.x , bbox.Center().y , bbox.max.z);
      case 0b100:
        return glm::vec3(bbox.Center().x , bbox.max.y , bbox.max.z);

      case 0b011:
        return glm::vec3(bbox.max.x , bbox.Center().y , bbox.Center().z);
      case 0b101:
        return glm::vec3(bbox.Center().x , bbox.max.y , bbox.Center().z);
      case 0b110:
        return glm::vec3(bbox.Center().x , bbox.Center().y , bbox.max.z);

      default:
        OE_ASSERT(false , "Invalid location : {:>03b}" , location);
    }
  }
  
  void Octant::ExpandAround(const glm::vec3& point) {
    if (bbox.Contains(point)) {
      OE_TRACE(" > Point {} already contained in octant {}" , point , tree_index);
      return;
    }

    OE_TRACE(" > Expanding octant {} to include point {}" , tree_index , point);
    bbox.ExpandToInclude(point);

    glm::vec3 min = bbox.min;
    glm::vec3 max = bbox.max;
    for (size_t i = 0; i < kNumChildren; ++i) {
      auto& c = children[i];
      min = glm::min(min , c->bbox.min);
      max = glm::max(max , c->bbox.max);
    }
    bbox = BoundingBox(min , max);
  }

  Octant::writer& Octant::writer::operator<<(const std::string& msg) {
    std::ranges::for_each(std::views::iota(0u , indent_stack) , [this](uint32_t) { os << "  "; });
    os << msg;
    return *this;
  }
  
  void Octant::writer::increment() { 
    indent_stack++;
  }
  
  void Octant::writer::decrement() { 
    indent_stack--;
  }

  void Octant::Serialize(std::ostream& os , bool print_children) const {
    writer w(os , 0);
    Serialize(w , print_children);
  }

  using namespace std::string_literals;
  void Octant::Serialize(writer& w , bool print_children) const {
    w <<  fmtstr("[Octant [{}] = {:p}\n" , tree_index , static_cast<const void*>(this));
    w.increment();

    w << fmtstr("(@{})\n" , bbox);

    auto origin = bbox.Center();
    w << fmtstr("(@Depth = {} , @Partition = {} , @Location = {:>03b} , @Dimensions = ({}) , @Center = ({}))\n", 
                depth , partition_index , partition_location , bbox.extent - origin , origin);
    std::stringstream ss;
    if (entities.empty()) {
      w << "(@Entities = 0)\n"s;
    } else {
      w << fmtstr("(@Entities = {}\n" , entities.size());
      for (const auto& e : entities) {
        w << fmtstr(" - {}\n" , e->Name()); 
      }
      w << ")\n"s;
    }
    
    if (IsLeaf()) {
      w << "(@Children = 0)\n"s;
    } else {
      w << fmtstr("(@Children = {}\n" , kNumChildren);
    }
    
    if (print_children && !IsLeaf()) {
      for (size_t i = 0; i < kNumChildren; ++i) {
        w << fmtstr(" - [{}] = {} [{}]\n", i , children[i]->tree_index , static_cast<void*>(children[i]));
      }
      w << ")\n"s;

      for (const auto& c : children) {
        if (c != nullptr) {
          c->Serialize(w , true);
        }
      }
    }
    
    w.decrement();
    w << "]\n"s;
  }  
  
  void Octant::Subdivide(StableVector<Octant>& octants) {
    for (size_t i = 0; i < kNumChildren; ++i) {
      auto [idx , child] = octants.EmplaceBackNoLock();
      child.depth = depth - 1;
      child.tree_index = idx;
      child.partition_index = i;
      child.partition_location = kOctantLocations[i];
      child.parent = this;

      glm::vec3 bbox_min = GetMinForSubQuadrant(child.partition_location);
      glm::vec3 bbox_max = GetMaxForSubQuadrant(child.partition_location);
      child.bbox = BoundingBox(bbox_min , bbox_max);
      children[i] = &octants[idx];
    } 
  }

  Octree::Octree()
      : depth(0), num_octants(NumOctantsAtDepth(0)) {
    Initialize(glm::vec3{ 0.f });
  }

  Octree::Octree(const glm::vec3& space_dimensions) 
      : depth(0) , num_octants(NumOctantsAtDepth(0)) {
    Initialize(space_dimensions);
  }
  
  Octree::Octree(const glm::vec3& space_dimensions , size_t resolution) 
      : depth(resolution) , num_octants(NumOctantsAtDepth(resolution)) {
    Initialize(space_dimensions);
  }

  Octree::~Octree() {
    octants.Clear();
  }
      
  const size_t Octree::Depth() const { 
    return depth;
  }
  
  const size_t Octree::NumOctants() const { 
    return octants.Size();
  }
  
  Octant& Octree::GetSpace() {
    OE_ASSERT(space != nullptr , "Space is null!");
    return *space;
  }

  const Octant& Octree::GetSpace() const {
    OE_ASSERT(space != nullptr , "Space is null!");
    return *space;
  }
  
  glm::vec3 Octree::Dimensions() const { 
    OE_ASSERT(space != nullptr , "Space is null!");
    return space->bbox.extent - space->bbox.Center(); 
  }
  
  void Octree::Subdivide(size_t depth) {
    OE_ASSERT(space != nullptr , "Space is null!");
    space->Subdivide(octants , depth);
  }

  bool Octree::Contains(const glm::vec3& point) const {
    OE_ASSERT(space != nullptr , "Space is null!");
    return space->Contains(point);
  }

  Octant& Octree::FindOctant(const glm::vec3& point) {
    OE_ASSERT(space != nullptr , "Space is null!");
    return space->FindOctant(point);
  }
  
  Octant& Octree::FindOctant(uint8_t location , size_t at_depth) {
    OE_ASSERT(space != nullptr , "Space is null!");
    return space->FindOctant(location , at_depth);
  }

  Octant& Octree::FindFurthestOctant(uint8_t location) {
    OE_ASSERT(space != nullptr , "Space is null!");
    return space->FindFurthestOctant(location);
  }

  void Octree::PrintOctants(std::ostream& os) const {
    PrintOctant(os , GetSpace());
  }
  
  void Octree::PrintOctant(std::ostream& os , const Octant& octant) const {
    octant.Serialize(os);
  }

  void Octree::ExpandToInclude(const glm::vec3& point) {
    OE_ASSERT(space != nullptr , "Space is null!");
    if (Contains(point)) {
      return;
    }

    if (space->IsLeaf()) {
      OE_ASSERT(octants.Size() == 1 , "Invalid number of octants in octree : {}" , octants.Size());
      space->ExpandToInclude(point);
      space->Subdivide(octants , 1);
    }
    space->ExpandToInclude(point);
  }
  
  void Octree::AddScene(Ref<Scene> scene , const glm::vec3& position) {
    OE_ASSERT(scene != nullptr , "Scene is null!");
    ExpandToInclude(position);

    OE_DEBUG("Adding scene to octree at <{}>" , position);
    scene->ForEachEntity([&](Entity* entity) {
      const auto& transform = entity->ReadComponent<Transform>(); 
      AddEntity(entity , position + transform.position);
    });
  }

  void Octree::AddEntity(Entity* entity , const glm::vec3& global_pos) {
    OE_ASSERT(entity != nullptr , "Attempting to add null entity to octree");
    OE_DEBUG("Adding entity : {} to octree at <{}>" , entity->Name() , global_pos);
    
    if (!Contains(global_pos)) {
      ExpandToInclude(global_pos);
    }

    uint8_t location = LocationFromPoint(global_pos);
    OE_TRACE(" > Entity {}@{} is contained in octree space [{:>03b}]" , entity->Name() , global_pos , location);
    {
      Octant& octant = GetSpace().FindFurthestOctant(location);
      if (octant.tree_index > 0) {
        OE_DEBUG(" > Adding : {}@{} C [{}]" , entity->Name() , global_pos , octant);
        octant.entities.push_back(entity);
        return;
      } 
    }

    OE_ASSERT(false , "Failed to add entity to octree!");
  }
  
  void Octree::Initialize(const glm::vec3& dim) {
    octants.Clear();
    dimensions = dim;

    {
      auto [tidx , s] = octants.EmplaceBackNoLock(); 
      s.depth = depth;
      s.tree_index = tidx;
      s.partition_index = 0;
      s.partition_location = 0b000;
      s.parent = nullptr;
      s.bbox = BoundingBox(-dim / 2.f , dim / 2.f);

      OE_TRACE("OTREE(@{})" , s.bbox);
      s.Subdivide(octants , depth);
    }
    
    OE_ASSERT(num_octants == NumOctantsAtDepth(depth) , "Invalid number of octants : {} != {}" , num_octants , NumOctantsAtDepth(depth));
    OE_ASSERT(octants.Size() == num_octants , "Invalid number of octants created : {} != {}" , octants.Size() , num_octants);
    space = &octants[0];
  } 

} // namespace other
