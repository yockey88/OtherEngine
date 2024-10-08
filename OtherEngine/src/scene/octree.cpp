/**
 * \file scene/octree.cpp
 **/
#include "scene/octree.hpp"

#include <algorithm>
#include <glm/fwd.hpp>
#include <ranges>

#include "core/logger.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/transform.hpp"

#include "scene/bounding_box.hpp"
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
    return std::ranges::all_of(children , [](Octant* octant) { return octant == nullptr; });
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

  int64_t Octant::GetMinDepth() const {
    if (IsLeaf()) {
      return 0;
    }
    auto depths = children | std::views::transform([](Octant* octant) -> int64_t { return octant->GetMinDepth(); });
    return std::ranges::min(depths) + 1;
  }

  int64_t Octant::GetMaxDepth() const {
    if (IsLeaf()) {
      return 0;
    }
    auto depths = children | std::views::transform([](Octant* octant) -> int64_t { return octant->GetMaxDepth(); });
    return std::ranges::max(depths) + 1;
  }

  glm::vec3 Octant::GlobalPosition() const {
    OE_ASSERT(embedding_space != nullptr , "Octant {} has no embedding space" , tree_index);
    return embedding_space->global_position + bbox.Center();
  }

  const std::vector<Entity*>& Octant::Entities() const {
    return entities;
  }

  void Octant::AddEntity(Entity* entity , const glm::vec3& position) {
    ExpandToInclude(position);
    OE_ASSERT(Contains(position) , "Octant {} does not contain point {}" , *this , position);

    if (IsLeaf()) {
      entities.push_back(entity);
      return;
    }

    uint8_t loc = LocationFromPoint(position);
    size_t idx = kLocationIndex.at(loc);
    auto* child = children[idx];
    if (child == nullptr) {
      SubdivideInDirection(loc , 1);
      child = children[idx];
    }

    child->AddEntity(entity , position);
  }

  static Octant null_octant;
  Octant& Octant::FindOctant(const glm::vec3& point) {
    if (!Contains(point)) {
      return null_octant;
    }

    if (IsLeaf()) {
      return *this;
    }

    uint8_t loc = LocationFromPoint(point);
    auto* child = children[kLocationIndex.at(loc)];
    if (child == nullptr) {
      return null_octant;
    }

    return child->FindOctant(point);
  }
  
  Octant& Octant::FindOctant(uint8_t location , size_t at_depth) {
    return *GetOctant(location , at_depth);
  }

  Octant& Octant::FindFurthestOctant(uint8_t location) {
    return *GetFurthestOctant(location);
  }

  void Octant::ExpandToInclude(const glm::vec3& point) {
    bbox.ExpandToInclude(point);
    if (IsLeaf() || !Contains(point)) {
      return;
    }
    
    uint8_t loc = LocationFromPoint(point);
    auto* child = children[kLocationIndex.at(loc)];
    if (child == nullptr) {
      return;
    }

    glm::vec3 min = Min();
    glm::vec3 max = Max();

    child->ExpandToInclude(point);
    if (vec3_lt(child->Min() , Min())) {
      min = child->Min();
    }
    if (vec3_lt(max , child->Max())) {
      max = child->Max();
    }

    bbox = BoundingBox(min , max);
  }

  void Octant::Subdivide(size_t d) {
    OE_ASSERT(embedding_space != nullptr , "Octant {} has no embedding space" , tree_index);
    if (d == 0) {
      std::ranges::fill(children , nullptr);
      return;
    }

    Subdivide();
    for (auto& child : children) {
      if (child == nullptr) {
        continue;
      }

      child->Subdivide(d - 1);
    }
  }

  void Octant::SubdivideInDirection(uint8_t direction , size_t depth) {
    if (depth == 0) {
      return;
    }

    int64_t loc_idx = kLocationIndex.at(direction);
    auto* child = children[loc_idx];
    if (child == nullptr) {
      child = children[loc_idx] = CreateChild(loc_idx);
    }

    if (depth == 1) {
      return;
    }

    child->SubdivideInDirection(direction , depth - 1);
  }
  
  Octant* Octant::GetChild(const glm::vec3& point) {
    if (!Contains(point)) {
      return nullptr;
    }

    if (IsLeaf()) {
      return this;
    }
    
    uint8_t loc = LocationFromPoint(point);
    auto* child = children[kLocationIndex.at(loc)];
    if (child == nullptr) {
      return nullptr;
    }

    return child->GetChild(point);
  }
  
  Octant* Octant::GetOctant(uint8_t location , size_t at_depth) {
    if (IsLeaf() || at_depth == 0) {
      return this;
    }

    auto idx = kLocationIndex.at(location);
    auto* child = children[idx];
    if (child == nullptr) {
      return this;
    }

    return child->GetOctant(location , at_depth - 1);
  }

  Octant* Octant::GetFurthestOctant(uint8_t location) {
    if (IsLeaf()) {
      return this;
    }

    auto idx = kLocationIndex.at(location);
    auto* child = children[idx];
    if (child == nullptr) {
      return this;
    }

    return child->GetFurthestOctant(location);
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
    w << fmtstr("(@Location({:>03b} , {}) @Dimensions{}  @Center{})\n", 
                  partition_location , partition_index  , bbox.extent , bbox.Center());

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
        auto* c = children[i];
        if (c == nullptr) {
          w << fmtstr(" - [{}] = null\n", i);
        } else {
          w << fmtstr(" - [{}] = {} [{}]\n", i , c->tree_index , static_cast<void*>(c));
        }
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
  
  void Octant::Subdivide() {
    OE_ASSERT(embedding_space != nullptr , "Embedding space is null!");
    for (size_t i = 0; i < kNumChildren; ++i) {
      SubdivideInDirection(i );
    } 
  }

  Octant* Octant::CreateChild(int64_t index) {
    auto [idx , child] = embedding_space->octants.EmplaceBackNoLock();
    child.tree_index = idx;
    child.embedding_space = embedding_space;
    child.parent = this;
    child.partition_index = index;
    child.partition_location = kOctantLocations[index];

    glm::vec3 bbox_min = GetMinForSubQuadrant(child.partition_location);
    glm::vec3 bbox_max = GetMaxForSubQuadrant(child.partition_location);
    child.bbox = BoundingBox(bbox_min , bbox_max);
    return &embedding_space->octants[idx];
  }

  Octree::Octree(const glm::vec3& origin)
      : global_position(origin) , depth(0), num_octants(NumOctantsAtDepth(0)) {
    Initialize(glm::zero<glm::vec3>());
  }

  Octree::~Octree() {
    octants.Clear();
  }
      
  const size_t Octree::Depth() const { 
    return GetSpace().GetMaxDepth();
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
    return GetSpace().bbox.extent;
  }
  
  void Octree::Subdivide(size_t depth) {
    Initialize(Dimensions());
    GetSpace().Subdivide(depth);
  }

  bool Octree::Contains(const glm::vec3& point) const {
    return GetSpace().Contains(point);
  }
  
  void Octree::MoveOriginTo(const glm::vec3& origin) {
    global_position = origin;
  }
  
  Octant& Octree::GetContainingOctant(const glm::vec3& point) {
    if (vec3_eq(point , global_position)) {
      return *space;
    } else {
      return GetSpace().FindOctant(point);
    }
  }

  Octant& Octree::FindOctant(const glm::vec3& point) {
    return GetSpace().FindOctant(point);
  }
  
  Octant& Octree::FindOctant(uint8_t location , size_t at_depth) {
    return GetSpace().FindOctant(location , at_depth);
  }

  Octant& Octree::FindFurthestOctant(uint8_t location) {
    return GetSpace().FindFurthestOctant(location);
  }

  void Octree::PrintOctants(std::ostream& os) const {
    PrintOctant(os , GetSpace());
  }
  
  void Octree::PrintOctant(std::ostream& os , const Octant& octant) const {
    octant.Serialize(os);
  }

  void Octree::ExpandToInclude(const glm::vec3& point) {
    GetSpace().ExpandToInclude(point);
  }
  
  void Octree::AddScene(Ref<Scene> scene , const glm::vec3& position) {
    OE_ASSERT(scene != nullptr , "Scene is null!");

    auto& entities = scene->SceneEntities();
    for (auto& [id , ent] : entities) {
      AddEntity(ent , ent->ReadComponent<Transform>().position);
    }
  }

  void Octree::AddEntity(Entity* entity , const glm::vec3& global_pos) {
    OE_ASSERT(entity != nullptr , "Attempting to add null entity to octree");
    
    /// TODO: make coordinates local to octree
    // glm::vec3 local_pos = global_pos - global_position;
    GetSpace().AddEntity(entity , global_pos);
  }
  
  void Octree::Initialize(const glm::vec3& dim) {
    octants.Clear();
    dimensions = dim;

    {
      auto [tidx , s] = octants.EmplaceBackNoLock(); 
      s.embedding_space = this;
      s.tree_index = tidx;
      s.partition_index = 0;
      s.partition_location = 0b000;
      s.parent = nullptr;

      if (vec3_eq(dim , glm::zero<glm::vec3>())) {
        s.bbox = BoundingBox();
      } else {
        /// TODO:
        /// take into account octree's global position
        s.bbox = BoundingBox(-dim / 2.f , dim / 2.f);
      }


      OE_DEBUG("OCTREE(@{} @total-depth({}) @dimension{})" , s.bbox , s.bbox.Center() , dim);
      s.Subdivide(depth);
    }
    
    OE_ASSERT(num_octants == NumOctantsAtDepth(depth) , "Invalid number of octants : {} != {}" , num_octants , NumOctantsAtDepth(depth));
    OE_ASSERT(octants.Size() == num_octants , "Invalid number of octants created : {} != {}" , octants.Size() , num_octants);
    space = &octants[0];
  } 

} // namespace other
