/**
 * \file scene/bvh_node.hpp
 **/
#ifndef OTHER_ENGINE_BVH_NODE_HPP
#define OTHER_ENGINE_BVH_NODE_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string_view>
#include <vector>
#include <ranges>

#ifdef OE_TESTING_ENVIRONMENT
#include <gtest/gtest.h>
#endif

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <spdlog/fmt/fmt.h>

#include <magic_enum/magic_enum.hpp>

#include <core/stable_vector.hpp>

#include "core/ref.hpp"

#include "math/vecmath.hpp"
#include "math/bounding_box.hpp"

#include "asset/asset_manager.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/transform.hpp"
#include "scene/scene.hpp"

#include "rendering/scene_renderer.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/pipeline.hpp"


namespace other {

  constexpr static size_t kNumDimensions = 3;
  
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

  enum BvhChildIdx {
    OCTANT1 = 0 , /// <+,+,+>
    OCTANT2 = 1 , /// <-,+,+>
    OCTANT3 = 2 , /// <-,-,+>
    OCTANT4 = 3 , /// <+,-,+>
    OCTANT5 = 4 , /// <+,-,->
    OCTANT6 = 5 , /// <-,+,->
    OCTANT7 = 6 , /// <-,-,->
    OCTANT8 = 7 , /// <+,-,->

    LEFT = 0 ,
    RIGHT = 1 ,
  };

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

  constexpr std::array<uint8_t , kNumCubeCorners> kOctantLocations = {
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

  class Entity;

  using dotother::StableVector;

  enum BvhPartitionAlgorithm {
    OCTREE = 0 ,
    HLBVH ,
  };

} // namespace other

template <>
struct fmt::formatter<other::BvhChildIdx> : public fmt::formatter<std::string_view> {
  public:
    auto format(other::BvhChildIdx& idx , fmt::format_context& ctx) {
      std::string_view name = magic_enum::enum_name(idx);
      return fmt::formatter<std::string_view>::format(name , ctx);
    }
};

template <>
struct fmt::formatter<other::BvhPartitionAlgorithm> : public fmt::formatter<std::string_view> {
  public:
    auto format(other::BvhPartitionAlgorithm& algo , fmt::format_context& ctx) {
      std::string_view name = magic_enum::enum_name(algo);
      return fmt::formatter<std::string_view>::format(name , ctx);
    }
};

namespace other {

  template <size_t N , BvhPartitionAlgorithm A>
  class Bvh;

  template <size_t N , BvhPartitionAlgorithm A>
  class BvhNode {
    public:
      constexpr static uint8_t LocationFromPoint(const glm::vec3& point) {
        uint8_t location = 0;
        location |= (point.x >= 0.f ? 0 : 1) << 2;
        location |= (point.y >= 0.f ? 0 : 1) << 1;
        location |= (point.z >= 0.f ? 0 : 1);
        return location;
      }

      constexpr static uint8_t LocationFromPoint(const glm::vec3& point , const glm::vec3& center , const glm::vec3& dimensions) {
        /// translate to global origin
        glm::vec3 translated = point - (center - dimensions / 2.f);
        return LocationFromPoint(translated);
      }

      bool IsLeaf() const {
        return std::ranges::all_of(children , [](BvhNode* octant) { return octant == nullptr; });
      }

      const std::array<BvhNode* , N>& Children() const { 
        return children; 
      }

      bool Contains(const glm::vec3& point) const { 
        return bbox.Contains(point); 
      }
      
      const glm::vec3& Min() const { 
        return bbox.min; 
      }

      const glm::vec3& Max() const { 
        return bbox.max; 
      }

      /// TODO: FIGURE OUT HOW TO USE RANGE ADAPTORS
      // struct DepthCounter : std::ranges::range_adaptor_closure<DepthCounter> {
      //   constexpr uint64_t operator()(const std::array<BvhNode<N , A>*>& children) const {
      //     return 0;
      //   }
      // };

      int64_t GetMinDepth() const {
        if (IsLeaf()) {
          return 0;
        }

        // int64_t min_depth = std::numeric_limits<int64_t>::max();
        // for (const auto& child : children) {
        //   if (child != nullptr) {
        //     min_depth = std::min(min_depth , child->GetMinDepth() + 1);
        //   }
        // }
        // return min_depth;

        auto depths = children | 
          std::views::filter([](BvhNode<N , A>* child) { return child != nullptr; }) |
          std::views::transform([](BvhNode<N , A>* child) -> int64_t { return child->GetMinDepth() + 1; });
        return std::ranges::min(depths);
      }

      int64_t GetMaxDepth() const {
        if (IsLeaf()) {
          return 0;
        }

        // int64_t max = std::numeric_limits<int64_t>::min();
        // for (const auto& child : children) {
        //   if (child != nullptr) {
        //     max = std::max(max , child->GetMaxDepth() + 1);
        //   }
        // }
        // return max;

        auto depths = children | 
          std::views::filter([](BvhNode<N , A>* child) { return child != nullptr; }) |
          std::views::transform([](BvhNode<N , A>* child) -> int64_t { return child->GetMaxDepth() + 1; });
        return std::ranges::max(depths);
      }

      glm::vec3 GlobalPosition() const {
        return global_position + bbox.Center();
      }

      const std::vector<Entity*>& Entities() const {
        return entities;
      }

      BvhNode<N , A>& FindNode(const glm::vec3& point) {
        return *GetNode(point);
      }

      BvhNode<N , A>& FindNode(uint8_t location , size_t at_depth) {
        return *GetNode(location , at_depth);
      }

      BvhNode<N , A>& FindFurthestNode(uint8_t location) {
        return *GetFurthestNode(location);
      }
      
      struct writer : public std::ostream {
        std::ostream& os;
        uint32_t indent_stack = 0;

        writer(std::ostream& os , uint32_t indent_stack)
            : std::ostream(os.rdbuf()) , os(os) , indent_stack(indent_stack) {}

        writer& operator<<(const std::string& msg) {
          std::ranges::for_each(std::views::iota(0u , indent_stack) , [this](uint32_t) { os << "  "; });
          os << msg;
          return *this;
        }

        void increment() { 
          indent_stack++;
        }
        
        void decrement() { 
          indent_stack--;
        }
      };
      
      void Serialize(std::ostream& os , bool print_children = false) const {
        writer w(os , 0);
        Serialize(w , print_children);
      }
      
      void AddScene(Ref<Scene>& scene , const glm::vec3& position) {
        OE_ASSERT(scene != nullptr , "Scene is null!");

        auto& entities = scene->SceneEntities();
        for (auto& [id , ent] : entities) {
          AddEntity(ent , ent->ReadComponent<Transform>().position);
        }

        if constexpr (N == 2) {
          OE_ASSERT(tree != nullptr , "Tree is null!");
          RebuildTree(*tree);
        }
      }
      
      void AddEntity(Entity* entity , const glm::vec3& position) {
        if constexpr (N == 8) {
          ExpandToInclude(position);
          OE_ASSERT(Contains(position) , "Octant {} does not contain point {}" , *this , position);
        }

        /// Call specialized tree insertion function with necessary data 
        ///     loc goes unused for 2 child nodes
        uint8_t loc = LocationFromPoint(position);
        InsertEntity(entity , position , loc);
      }

      void Rebuild() {
        if constexpr (N == 2) {
          RebuildTree(*tree);
        }
      }

      void ExpandToInclude(const glm::vec3& point);

      void Subdivide(size_t depth);
      
      void RenderEntityBounds(const std::string_view pl_name , Ref<SceneRenderer>& renderer , bool outline);
      void RenderNodeBounds(const std::string_view pl_name , Ref<SceneRenderer>& renderer , size_t depth);
      
      int64_t tree_index = -1;
      
      int64_t partition_index = -1;
      uint8_t partition_location = 0b000;

      BBox bbox = BBox();

      Bvh<N , A>* tree = nullptr;
      BvhNode<N , A>* parent = nullptr;

      StableVector<BvhNode<N , A>>* nodes = nullptr;
      glm::vec3 global_position{ 0.f };

      bool built = false;

    private:
      std::array<BvhNode<N , A>*, N> children;

      void InsertEntity(Entity* entity , const glm::vec3& position , uint8_t location);

      BvhNode<N , A>* GetNode(const glm::vec3& point);
      BvhNode<N , A>* GetNode(uint8_t location , size_t at_depth);
      BvhNode<N , A>* GetFurthestNode(uint8_t location);

      glm::vec3 GetMinForSubQuadrant(uint8_t location);
      glm::vec3 GetMaxForSubQuadrant(uint8_t location);

      void SubdivideInDirection(uint8_t direction , size_t depth = 1);
      void SubdivideChild(BvhChildIdx location , const glm::vec3& scale , const glm::vec3& center);
      void Subdivide();
      void Serialize(writer& w , bool children) const;
      BvhNode<N , A>* CreateChild(int64_t index);

      std::vector<Entity*> entities = {};

      static BvhNode<N , A>& RebuildTree(Bvh<N , A>& tree) {
        OE_ASSERT(N == 2 , "RebuildTree only works with 2D trees!");
        OE_ASSERT(tree.space != nullptr , "Root is not a leaf node!");

        std::vector<Entity*> entities{};
        entities.swap(tree.space->entities);

        glm::vec3 dim = tree.Dimensions();
        tree.Initialize(dim);
        OE_ASSERT(tree.space != nullptr , "Failed to reinitialize root node!");

        tree.space = RebuildTree(tree.space , entities);
        glm::vec3 min = tree.space->bbox.min;
        glm::vec3 max = tree.space->bbox.max;
        OE_DEBUG("Rebuilt tree {} , {}" , max , min);
      }

      static bool NeedsRebuild(BvhNode<N , A>* space , const std::vector<Entity*>& entities);
      static BvhNode<N , A>* TreeFromSortedList(BvhNode<2 , HLBVH>* space , const std::vector<BvhNode<N , A>*>& nodes);
      static BvhNode<N , A>* RebuildTree(BvhNode<N , A>* space , std::vector<Entity*>& entities);

      friend class Bvh<N , A>;

#ifdef OE_TESTING_ENVIRONMENT
      friend class OtherTest;
      FRIEND_TEST(OctreeTests , depth_0);
      FRIEND_TEST(OctreeTests , higher_res_2);
      FRIEND_TEST(OctreeTests , higher_res_3);
#endif
  };

  template <size_t N , BvhPartitionAlgorithm A>
  void BvhNode<N , A>::Subdivide(size_t d) {
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

  template <size_t N , BvhPartitionAlgorithm A>
  BvhNode<N , A>* BvhNode<N , A>::GetNode(const glm::vec3& point) {
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

    return child->GetNode(point);
  }
  
  template <size_t N , BvhPartitionAlgorithm A>
  BvhNode<N , A>* BvhNode<N , A>::GetNode(uint8_t location , size_t at_depth) {
    if (IsLeaf() || at_depth == 0) {
      return this;
    }

    auto idx = kLocationIndex.at(location);
    auto* child = children[idx];
    if (child == nullptr) {
      return this;
    }

    return child->GetNode(location , at_depth - 1);
  }

  template <size_t N , BvhPartitionAlgorithm A>
  BvhNode<N , A>* BvhNode<N , A>::GetFurthestNode(uint8_t location) {
    if (IsLeaf()) {
      return this;
    }

    auto idx = kLocationIndex.at(location);
    auto* child = children[idx];
    if (child == nullptr) {
      return this;
    }

    return child->GetFurthestNode(location);
  }
  
  template <size_t N , BvhPartitionAlgorithm A>
  glm::vec3 BvhNode<N , A>::GetMinForSubQuadrant(uint8_t location) {
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

  template <size_t N , BvhPartitionAlgorithm A>
  glm::vec3 BvhNode<N , A>::GetMaxForSubQuadrant(uint8_t location) {
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

  template <size_t N , BvhPartitionAlgorithm A>
  void BvhNode<N , A>::RenderEntityBounds(const std::string_view pl_name , Ref<SceneRenderer>& renderer , bool outline) {
    const static AssetHandle wireframe = ModelFactory::CreateBoxWireframe();
    Ref<StaticModel> model = AssetManager::GetAsset<StaticModel>(wireframe);
    Material mat = {
      .color = glm::vec4(0.f , 1.f , 0.f , 1.f) ,
      .shininess = 16.f ,
    };
    
    for (const auto& e : entities) {
      if (e->visited) {
        continue;
      }

      RenderSubmission s = {
        .model = model ,
        .transform = e->GetComponent<Transform>().CalcMatrix() ,
        .material = mat ,
        .render_state = RenderState::FILL ,
        .draw_mode = DrawMode::LINES ,
      };
      renderer->SubmitStaticModel(pl_name , s);

      e->visited = true;
    }

    if (IsLeaf()) {
      return;
    }

    for (const auto& c : children) {
      if (c != nullptr) {
        c->RenderEntityBounds(pl_name , renderer , outline);
      }
    }
  }
  
  template <size_t N , BvhPartitionAlgorithm A>
  void BvhNode<N , A>::RenderNodeBounds(const std::string_view pl_name , Ref<SceneRenderer>& renderer , size_t depth) {
    constexpr glm::mat4 identity = glm::identity<glm::mat4>();
    const static AssetHandle wireframe = ModelFactory::CreateBoxWireframe();

    Ref<StaticModel> model = AssetManager::GetAsset<StaticModel>(wireframe);
    Material mat = {
      .color = glm::vec4(1.f , 0.f , 0.f , 1.f) ,
      .shininess = 16.f ,
    };

    glm::mat4 model_mat = glm::translate(identity , bbox.Center());
    model_mat = glm::scale(model_mat , bbox.extent);
    if constexpr (N == 2) {
      model_mat = glm::scale(model_mat , glm::vec3(1.3f));
    }

    RenderSubmission s = {
      .model = model ,
      .transform = model_mat ,
      .material = mat ,
      .render_state = RenderState::FILL ,
      .draw_mode = DrawMode::LINES ,
    };

    renderer->SubmitStaticModel(pl_name , s);

    if constexpr (N == 8) {
      if (IsLeaf() || depth == 0) {
        return;
      }

      for (const auto& c : children) {
        if (c != nullptr) {
          c->RenderNodeBounds(pl_name , renderer , depth - 1);
        }
      }
    } else if constexpr (N == 2) {
      if (children[LEFT] != nullptr) {
        children[LEFT]->RenderNodeBounds(pl_name , renderer , depth - 1);
      }
      
      if (children[RIGHT] != nullptr) {
        children[RIGHT]->RenderNodeBounds(pl_name , renderer , depth - 1);
      }
    }

  }

  template <size_t N , BvhPartitionAlgorithm A>
  void BvhNode<N , A>::Serialize(writer& w , bool print_children) const {
    using namespace std::string_literals;
    w <<  fmtstr("[BvhNode [{}] = {:p}\n" , tree_index , static_cast<const void*>(this));
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
      w << fmtstr("(@Children = {}\n" , N);
    }
    
    if (print_children && !IsLeaf()) {
      for (size_t i = 0; i < N; ++i) {
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
  
  template <size_t N , BvhPartitionAlgorithm A>
  void BvhNode<N , A>::SubdivideInDirection(uint8_t direction , size_t depth) {
    if (depth == 0) {
      return;
    }

    int64_t loc_idx = kLocationIndex.at(direction);
    BvhNode<N , A>* child = children[loc_idx];
    if (child == nullptr) {
      child = children[loc_idx] = CreateChild(loc_idx);
    }

    if (depth == 1) {
      return;
    }

    child->SubdivideInDirection(direction , depth - 1);
  }

  template <size_t N , BvhPartitionAlgorithm A>
  void BvhNode<N , A>::SubdivideChild(BvhChildIdx location , const glm::vec3& scale , const glm::vec3& center) {
    BvhNode<N , A>* child = children[location];
    OE_ASSERT(child == nullptr , "Child already exists at location {}" , location);

    child = children[location] = CreateChild(location);
    child->bbox = BBox(center - scale / 2.f , center + scale / 2.f);
    child->global_position = global_position + child->bbox.Center();
  } 

  template <size_t N , BvhPartitionAlgorithm A>
  void BvhNode<N , A>::Subdivide() {
    if constexpr (N == 8) {
      OE_ASSERT(A == OCTREE , "Octree partitioning only works with 8 children");
      for (size_t i = 0; i < N; ++i) {
        SubdivideInDirection(i );
      } 
    } else if constexpr (N == 2) {
      OE_ASSERT(false , "Cannot force subdivide BVH<2>!");
    }
  }

  template <size_t N , BvhPartitionAlgorithm A>
  BvhNode<N , A>* BvhNode<N , A>::CreateChild(int64_t index) {
    OE_ASSERT(nodes != nullptr , "Nodes is null!");

    auto [idx , child] = nodes->EmplaceBackNoLock();
    child.nodes = nodes;
    child.tree_index = idx;
    child.parent = this;
    child.partition_index = index;

    if constexpr (N == 8) {
      child.partition_location = kOctantLocations[index];

      glm::vec3 bbox_min = GetMinForSubQuadrant(child.partition_location);
      glm::vec3 bbox_max = GetMaxForSubQuadrant(child.partition_location);
      child.bbox = BBox(bbox_min , bbox_max);
      child.global_position = global_position + child.bbox.Center();
    } else if constexpr(N == 2) {
      if (index == LEFT) {
        child.partition_location = 0b000;
      } else {
        child.partition_location = 0b001;
      }
    } else {
      OE_ASSERT(false , "Invalid number of children : {}" , N);
    }

    return &(*nodes)[idx];
  }

} // namespace other

template <size_t N  , other::BvhPartitionAlgorithm A>
struct fmt::formatter<other::BvhNode<N , A>> : fmt::formatter<std::string_view> {
  auto format(const other::BvhNode<N , A>& octant , fmt::format_context& ctx) {
    std::stringstream ss;
    octant.Serialize(ss , false);
    return fmt::formatter<std::string_view>::format(ss.str() , ctx);
  }
};

constexpr inline fmt::formatter<other::BvhNode<8 , other::OCTREE>> oct_fmttr;

#endif // !OTHER_ENGINE_BVH_NODE_HPP