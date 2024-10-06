/**
 * \file scene/octree.hpp
 **/
#ifndef OTHER_ENGINE_OCTREE_HPP
#define OTHER_ENGINE_OCTREE_HPP

#include <array>
#include <ostream>
#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "core/defines.hpp"
#include "core/ref_counted.hpp"

#include "scene/bounding_box.hpp"

namespace other {

  class Entity;

  constexpr static size_t kNumDimensions = 3;
  constexpr static size_t kNumChildren = 8;
  constexpr static size_t kDepth = 3;

  /// octant 0 = +x, +y, +z
  /// octant 1 = -x, +y, +z
  /// octant 2 = -x, -y, +z
  /// octant 3 = +x, -y, +z
  /// octant 4 = +x, -y, -z 
  /// octant 5 = -x, -y, -z
  /// octant 6 = -x, +y, -z
  /// octant 7 = +x, +y, -z
  constexpr static size_t kPxPyPz = 0;
  constexpr static size_t kNxPyPz = 1;
  constexpr static size_t kNxNyPz = 2;
  constexpr static size_t kPxNyPz = 3;
  constexpr static size_t kPxNyNz = 4;
  constexpr static size_t kNxNyNz = 5;
  constexpr static size_t kNxPyNz = 6;
  constexpr static size_t kPxPyNz = 7;

  constexpr static size_t kNegativeXBit = 0b100;
  constexpr static size_t kNegativeYBit = 0b010;
  constexpr static size_t kNegativeZBit = 0b001;

  /// these are in CCW order starting in the positive x,y,z octant
  constexpr static uint8_t kPxPyPzLoc = 0b000;
  constexpr static uint8_t kNxPyPzLoc = 0 | kNegativeXBit;
  constexpr static uint8_t kNxNyPzLoc = 0 | kNegativeXBit | kNegativeYBit;
  constexpr static uint8_t kPxNyPzLoc = 0                 | kNegativeYBit;
  constexpr static uint8_t kPxNyNzLoc = 0                 | kNegativeYBit | kNegativeZBit;
  constexpr static uint8_t kNxNyNzLoc = 0 | kNegativeXBit | kNegativeYBit | kNegativeZBit;
  constexpr static uint8_t kNxPyNzLoc = 0 | kNegativeXBit                 | kNegativeZBit;
  constexpr static uint8_t kPxPyNzLoc = 0                                 | kNegativeZBit;

  constexpr std::array<uint8_t , kNumChildren> kOctantLocations = {
    kPxPyPzLoc,
    kNxPyPzLoc,
    kNxNyPzLoc,
    kPxNyPzLoc,
    kPxNyNzLoc,
    kNxNyNzLoc,
    kNxPyNzLoc,
    kPxPyNzLoc
  };

  const static std::map<uint8_t , uint8_t> kOppositeOctant = {
    { kPxPyPzLoc , kNxNyNzLoc },
    { kNxNyNzLoc , kPxPyPzLoc },
    { kNxPyPzLoc , kPxNyNzLoc },
    { kPxNyNzLoc , kNxPyPzLoc },
    { kPxNyNzLoc , kNxPyPzLoc },
    { kNxPyPzLoc , kPxNyNzLoc },
    { kNxNyPzLoc , kPxPyNzLoc },
    { kPxPyNzLoc , kNxNyPzLoc }
  };

  const static std::map<uint8_t , size_t> kLocationIndex = {
    { kPxPyPzLoc , 0 },
    { kNxPyPzLoc , 1 },
    { kNxNyPzLoc , 2 },
    { kPxNyPzLoc , 3 },
    { kPxNyNzLoc , 4 },
    { kNxNyNzLoc , 5 },
    { kNxPyNzLoc , 6 },
    { kPxPyNzLoc , 7 }
  };

  struct Octant;

  struct Octant {
    BoundingBox bbox;

    size_t depth = 0;
    size_t tree_index = 0;
    
    size_t partition_index = 0;
    uint8_t partition_location = 0;

    /// relative to parent octant 
    ///  i.e. -> this.global_location = parent->location + this.location
    Octant* parent;

    glm::vec3 dimensions{ 0.f };
    glm::vec3 origin{ 0.f };

    std::vector<Entity*> entities{};
    std::array<Scope<Octant>, kNumChildren> children{};

    Octant()
      : parent(nullptr) {}
    Octant(Scope<Octant>& parent)
      : parent(parent.get()) {}

    ~Octant() {
      for (auto& c : children) {
        c = nullptr;
      }
    }
  };
  

  class Octree : public RefCounted {
    public:
      Octree();
      Octree(const glm::vec3& space_dimensions , size_t depth = kDepth);
      ~Octree();

      inline const size_t Depth() const;
      inline const size_t NumOctants() const;

      Scope<Octant>& GetSpace();
      const glm::vec3& Dimensions() const;

      const Scope<Octant>& GetOctant(const glm::vec3& point) const;

      void PrintOctants(std::ostream& os) const;
      void PrintOctant(std::ostream& os , const Scope<Octant>& octant) const;
      
      void AddEntity(Entity* entity);

    private:
      /// root
      Scope<Octant> space = nullptr;

      size_t depth = 0;
      const size_t num_octants = 0;
      glm::vec3 dimensions;

      uint32_t idx_generator = 0;

      void Initialize(const glm::vec3& dim);

      bool OctantContainsPoint(const Scope<Octant>& octant , const glm::vec3& point) const;
      const Scope<Octant>& FindOctant(const Scope<Octant>& octant , const glm::vec3& point) const;

      void Subdivide(Scope<Octant>& octant, size_t depth);
      void Subdivide(Scope<Octant>& octant);

      glm::vec3 CalculateOctantOrigin(const glm::vec3& parent_dimensions , const glm::vec3& parent_origin , uint8_t location);
      glm::vec3 CalculateOctantDimensions(const glm::vec3& parent_dimensions);
  };

} // namespace other

#endif // !OTHER_ENGINE_OCTREE_HPP
