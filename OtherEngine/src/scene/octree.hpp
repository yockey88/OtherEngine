/**
 * \file scene/octree.hpp
 **/
#ifndef OTHER_ENGINE_OCTREE_HPP
#define OTHER_ENGINE_OCTREE_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string_view>
#include <vector>
#include <map>

#ifdef OE_TESTING_ENVIRONMENT
#include <gtest/gtest.h>
#endif

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <spdlog/fmt/fmt.h>

#include <core/stable_vector.hpp>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "scene/bounding_box.hpp"

namespace other {

  class Entity;
  class Scene;

  constexpr static size_t kNumDimensions = 3;
  constexpr static size_t kNumChildren = 8;
  
  //// octant locations
  ///             x,y,z
  ///        0 = <+,+,+>
  ///        1 = <-,+,+>
  ///        2 = <-,-,+>
  ///        3 = <+,-,+>
  ///        4 = <+,-,-> 
  ///        5 = <-,-,->
  ///        6 = <-,+,->
  ///        7 = <+,+,->
  constexpr static size_t kPxPyPz = 0;
  constexpr static size_t kNxPyPz = 1;
  constexpr static size_t kNxNyPz = 2;
  constexpr static size_t kPxNyPz = 3;
  constexpr static size_t kPxNyNz = 4;
  constexpr static size_t kNxNyNz = 5;
  constexpr static size_t kNxPyNz = 6;
  constexpr static size_t kPxPyNz = 7;

  /// negative x,y,z bits
  //                                        xyz 
  constexpr static size_t kNegativeXBit = 0b100;
  constexpr static size_t kNegativeYBit = 0b010;
  constexpr static size_t kNegativeZBit = 0b001;

  /// these are in CCW order starting in the positive x,y,z octant
  /// +z group of 4 ccw
  //                   0 = <+,+,+>
  //                   1 = <-,+,+>
  //                   2 = <-,-,+>
  //                   3 = <+,-,+>
  constexpr static uint8_t kPxPyPzLoc = 0b000;
  constexpr static uint8_t kNxPyPzLoc = 0b100;
  constexpr static uint8_t kNxNyPzLoc = 0b110;
  constexpr static uint8_t kPxNyPzLoc = 0b010;

  /// -z group of 4 ccw
  //                   4 = <+,+,->
  //                   5 = <-,+,->
  //                   6 = <-,-,->
  //                   7 = <+,-,-> 
  constexpr static uint8_t kPxPyNzLoc = 0b001;
  constexpr static uint8_t kNxPyNzLoc = 0b101;
  constexpr static uint8_t kNxNyNzLoc = 0b111;
  constexpr static uint8_t kPxNyNzLoc = 0b011;

  constexpr std::array<uint8_t , kNumChildren> kOctantLocations = {
    /// +z group of 4 ccw
    // 0 = <+,+,+>
    // 1 = <-,+,+>
    // 2 = <-,-,+>
    // 3 = <+,-,+>
    kPxPyPzLoc,
    kNxPyPzLoc,
    kNxNyPzLoc,
    kPxNyPzLoc,

    /// -z group of 4 ccw
    // 4 = <+,+,->
    // 5 = <-,+,->
    // 6 = <-,-,->
    // 7 = <+,-,-> 
    kPxPyNzLoc,
    kNxPyNzLoc,
    kNxNyNzLoc,
    kPxNyNzLoc,
  };

  const static std::map<uint8_t , uint8_t> kOppositeOctant = {
    /// +z group of 4 ccw
    // <+,+,+>     <-,-,->
    // <-,+,+>     <+,-,->
    // <-,-,+>     <+,+,->
    // <+,-,+>     <-,+,-> 
    { kPxPyPzLoc , kNxNyNzLoc },
    { kNxPyPzLoc , kPxNyNzLoc },
    { kNxNyPzLoc , kPxPyNzLoc },
    { kPxNyPzLoc , kNxPyNzLoc },

    /// -z group of 4 ccw
    // <+,+,->     <-,-,+>
    // <-,+,->     <+,-,+>
    // <-,-,->     <+,+,+>
    // <+,-,->     <-,+,+> 
    { kPxPyNzLoc , kNxNyPzLoc },
    { kNxPyNzLoc , kPxNyPzLoc },
    { kNxNyNzLoc , kPxPyPzLoc },
    { kPxNyNzLoc , kNxPyPzLoc }
  };

  const static std::map<uint8_t , size_t> kLocationIndex = {
    /// +z group of 4 ccw
    // 0 = <+,+,+>
    // 1 = <-,+,+>
    // 2 = <-,-,+>
    // 3 = <+,-,+>
    { /* 0b000 */ kPxPyPzLoc , 0 },
    { /* 0b100 */ kNxPyPzLoc , 1 },
    { /* 0b110 */ kNxNyPzLoc , 2 },
    { /* 0b010 */ kPxNyPzLoc , 3 },

    /// -z group of 4 ccw
    // 4 = <+,+,->
    // 5 = <-,+,->
    // 6 = <-,-,->
    // 7 = <+,-,-> 
    { /* 0b001 */ kPxPyNzLoc , 4 },
    { /* 0b101 */ kNxPyNzLoc , 5 },
    { /* 0b111 */ kNxNyNzLoc , 6 },
    { /* 0b011 */ kPxNyNzLoc , 7 },
  };

  using dotother::StableVector;

  class Octree;

  class Octant {
    public:
      int64_t tree_index = -1;
      
      int64_t partition_index = -1;
      uint8_t partition_location = 0b000;

      BoundingBox bbox = BoundingBox();

      Octree* embedding_space = nullptr;
      Octant* parent = nullptr;
      
      Octant()
         : tree_index(-1)  , parent(nullptr) {
        std::ranges::fill(children , nullptr);
      }
      ~Octant() {}
      
      bool IsLeaf() const; 

      const std::array<Octant* , kNumChildren>& Children() const;
      bool Contains(const glm::vec3& point) const;

      const glm::vec3& Min() const;
      const glm::vec3& Max() const;

      int64_t GetMinDepth() const;
      int64_t GetMaxDepth() const;

      glm::vec3 GlobalPosition() const;

      const std::vector<Entity*>& Entities() const;

      void AddEntity(Entity* entity , const glm::vec3& position);

      Octant& FindOctant(const glm::vec3& point);
      Octant& FindOctant(uint8_t location , size_t at_depth);
      Octant& FindFurthestOctant(uint8_t location);

      void ExpandToInclude(const glm::vec3& point);

      void Subdivide(size_t depth);
      void SubdivideInDirection(uint8_t direction , size_t depth = 1);

      void Serialize(std::ostream& os, bool print_children = true) const;
      struct writer : public std::ostream {
        std::ostream& os;
        uint32_t indent_stack = 0;

        writer(std::ostream& os , uint32_t indent_stack)
            : std::ostream(os.rdbuf()) , os(os) , indent_stack(indent_stack) {}

        writer& operator<<(const std::string& msg);
        void increment();
        void decrement();
      };
    private:
      std::array<Octant*, kNumChildren> children;

      Octant* GetChild(const glm::vec3& point);
      Octant* GetOctant(uint8_t location , size_t at_depth);
      Octant* GetFurthestOctant(uint8_t location);

      glm::vec3 GetMinForSubQuadrant(uint8_t location);
      glm::vec3 GetMaxForSubQuadrant(uint8_t location);

      void Subdivide();

      void Serialize(writer& w , bool children) const;

      Octant* CreateChild(int64_t index);

      std::vector<Entity*> entities = {};

#ifdef OE_TESTING_ENVIRONMENT
      friend class OtherTest;
      FRIEND_TEST(OctreeTests , depth_0);
      FRIEND_TEST(OctreeTests , higher_res_2);
      FRIEND_TEST(OctreeTests , higher_res_3);
#endif
  };
} // namespace other

template <>
struct fmt::formatter<other::Octant> : fmt::formatter<std::string_view> {
  auto format(const other::Octant& octant , fmt::format_context& ctx) {
    std::stringstream ss;
    octant.Serialize(ss , false);
    return fmt::formatter<std::string_view>::format(ss.str() , ctx);
  }
};

namespace other {

  class Octree : public RefCounted {
    public:
      Octree(const glm::vec3& origin = glm::vec3{0.f});
      ~Octree(); 

      const size_t Depth() const;
      const size_t NumOctants() const;
      Octant& GetSpace();
      const Octant& GetSpace() const;
      glm::vec3 Dimensions() const;

      std::array<glm::vec3 , kNumCubeCorners> Corners() const;

      void Subdivide(const glm::vec3& dimensions , size_t depth);
      bool Contains(const glm::vec3& point) const;

      void MoveOriginTo(const glm::vec3& origin);

      Octant& GetContainingOctant(const glm::vec3& point);

      Octant& FindOctant(const glm::vec3& point);
      Octant& FindOctant(uint8_t location , size_t at_depth);
      Octant& FindFurthestOctant(uint8_t location);

      void PrintOctants(std::ostream& os) const;
      void PrintOctant(std::ostream& os , const Octant& octant) const;

      void ExpandToInclude(const glm::vec3& point);

      /**
       * @param position - the global position of the origin of the scene
       **/
      void AddScene(Ref<Scene> scene , const glm::vec3& position);
      
      /**
       * @param global_pos - the global position of the entity
       **/
      void AddEntity(Entity* entity , const glm::vec3& global_pos);

    private:
      /// space == root, it is a single octant that represents the entire space
      constexpr inline static size_t space_index = 0;

      Octant* space = nullptr;
      StableVector<Octant> octants;

      size_t depth = 0;
      const size_t num_octants = 0;
      glm::vec3 global_position{ 0.f };

      void Initialize(const glm::vec3& dim);

      friend class Octant;

#ifdef OE_TESTING_ENVIRONMENT
      friend class OtherTest;
      FRIEND_TEST(OctreeTests , depth_0);
      FRIEND_TEST(OctreeTests , higher_res_2);
      FRIEND_TEST(OctreeTests , higher_res_3);
#endif
  };

} // namespace other

#endif // !OTHER_ENGINE_OCTREE_HPP
