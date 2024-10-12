/**
 * \file scene/bvh.hpp
 **/
#ifndef OTHER_ENGINE_BVH_HPP
#define OTHER_ENGINE_BVH_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
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

#include "math/math.hpp"
#include "math/ray.hpp"

#include "scene/bvh_node.hpp"

using dotother::StableVector;

namespace other {
  
  /// num nodes (children num = k) at depth n = k^0 + k^1 + k^2 + ... + k^n
  /// @TODO: maybe compute these tables at compile time for 8 , 2 , 4
  /** Node Num Table:
   *  0 -> k^0
   *  1 -> k^0 + k^1
   *  2 -> k^0 + k^1 + k^2
   *  3 -> k^0 + k^1 + k^2 + k^3
   *  4 -> k^0 + k^1 + k^2 + k^3 + k^4
   *  ....
   **/
  template <size_t N>
  constexpr static size_t NumNodesAtDepth(size_t depth) {
    size_t sum = 0;
    for (size_t i = 0; i <= depth; ++i) {
      sum += FastPow(N, i);
    }
    return sum;
  }

  template <size_t N , BvhPartitionAlgorithm A>
  class BvhNode;

  template <size_t N , BvhPartitionAlgorithm A>
  class Bvh : public RefCounted {
    public:
      Bvh(const glm::vec3& origin)
          : depth(0), num_nodes(NumNodesAtDepth<N>(0)) , global_position(origin) {
        Initialize(glm::zero<glm::vec3>());
      }

      ~Bvh() {
        nodes.Clear();
      }

      const size_t Depth() const { 
        return GetSpace().GetMaxDepth();
      }
      
      const size_t NumNodes() const { 
        return nodes.Size();
      }
      
      BvhNode<N , A>& GetSpace() {
        OE_ASSERT(space != nullptr , "Space is null!");
        return *space;
      }

      const BvhNode<N , A>& GetSpace() const {
        OE_ASSERT(space != nullptr , "Space is null!");
        return *space;
      }
      
      glm::vec3 Dimensions() const { 
        return GetSpace().bbox.extent;
      }
      
      std::array<glm::vec3 , N> Corners() const {
        return GetSpace().bbox.Corners();
      }

      void Subdivide(const glm::vec3& dim , size_t depth) {
        if constexpr (N != 8) {
          return;
        }

        if (dim == glm::zero<glm::vec3>()) {
          return;
        }

        Initialize(dim);
        GetSpace().Subdivide(depth);
      }
      
      void MoveOriginTo(const glm::vec3& origin) {
        global_position = origin;
      }
      
      BvhNode<N , A>& GetContainingNode(const glm::vec3& point) {
        if (vec3_eq(point , global_position)) {
          return *space;
        } else {
          return GetSpace().FindNode(point);
        }
      }

      BvhNode<N , A>& FindNode(const glm::vec3& point) {
        return GetSpace().FindNode(point);
      }
      
      BvhNode<N , A>& FindNode(uint8_t location , size_t at_depth) {
        return GetSpace().FindNode(location , at_depth);
      }

      BvhNode<N , A>& FindFurthestNode(uint8_t location) {
        return GetSpace().FindFurthestNode(location);
      }

      void PrintNodes(std::ostream& os) const {
        PrintNode(os , GetSpace());
      }
      
      void PrintNode(std::ostream& os , const BvhNode<N , A>& node) const {
        node.Serialize(os);
      }

      void ExpandToInclude(const glm::vec3& point) {
        GetSpace().ExpandToInclude(point);
      }

      Intersection Intersect(const Ray& ray) const {
        return GetSpace().Intersect(ray);
      }

      /**
       * @param position - the global position of the origin of the scene
       **/
      void AddScene(Ref<Scene>& scene , const glm::vec3& position) {
        OE_ASSERT(scene != nullptr , "Scene is null!");
        glm::vec3 local_pos = position - global_position;
        GetSpace().AddScene(scene , local_pos);
      }

      void Rebuild() {
        OE_ASSERT(N != 8 , "Rebuild not implemented for octrees!");
        GetSpace().RebuildTree(space , space->entities);
      }
      
      /**
       * @param global_pos - the global position of the entity
       **/
      void AddEntity(Entity* entity , const glm::vec3& global_pos) {
        OE_ASSERT(entity != nullptr , "Attempting to add null entity to octree");

        if constexpr (N == 2) {
          /// TODO: calculate dimension from scene bounds
          Initialize(glm::vec3{ 1.f });
        }
        glm::vec3 local_pos = global_pos - global_position;
        GetSpace().AddEntity(entity , local_pos);
      }

      void RenderEntityBounds(const std::string_view pl_name , Ref<SceneRenderer>& renderer , bool outline = true) {
        GetSpace().RenderEntityBounds(pl_name , renderer , outline);
      }

      void RenderBounds(const std::string_view pl_name , Ref<SceneRenderer>& renderer , Opt<size_t> depth = std::nullopt) {
        GetSpace().RenderNodeBounds(pl_name , renderer, depth.value_or(Depth() - 1));
      }

    private:
      /// space == root, it is a single node that represents the entire space
      constexpr inline static size_t space_index = 0;

      BvhNode<N , A>* space = nullptr;
      StableVector<BvhNode<N , A>> nodes;

      size_t depth = 0;
      const size_t num_nodes = 0;
      glm::vec3 global_position{ 0.f };

      void Initialize(const glm::vec3& dim);

      friend class BvhNode<N , A>;

#ifdef OE_TESTING_ENVIRONMENT
      friend class OtherTest;
      FRIEND_TEST(BvhTests , depth_0);
      FRIEND_TEST(BvhTests , higher_res_2);
      FRIEND_TEST(BvhTests , higher_res_3);
#endif
  };

  template <size_t N , BvhPartitionAlgorithm A>
  void Bvh<N , A>::Initialize(const glm::vec3& dim) {
    nodes.Clear();

    {
      auto [tidx , s] = nodes.EmplaceBackNoLock(); 
      s.tree = this;
      s.nodes = &nodes;
      s.global_position = global_position;
      // s.embedding_space = this;
      s.tree_index = tidx;
      s.partition_index = 0;
      s.partition_location = 0b000;
      s.parent = nullptr;

      if (vec3_eq(dim , glm::zero<glm::vec3>())) {
        s.bbox = BBox();
      } else {
        s.bbox = BBox(global_position - (dim / 2.f) , global_position + (dim / 2.f));
      }
      OE_DEBUG("BVH(@{} @total-depth({}) @dimension{})" , s.bbox , s.bbox.Center() , dim);
    }
    
    OE_ASSERT(num_nodes == NumNodesAtDepth<N>(depth) , "Invalid number of nodes : {} != {}" , num_nodes , NumNodesAtDepth<N>(depth));
    OE_ASSERT(nodes.Size() == num_nodes , "Invalid number of nodes created : {} != {}" , nodes.Size() , num_nodes);
    space = &nodes[0];
  } 

  using SceneBranch = Bvh<2 , HLBVH>;
  using BvhTree = Bvh<2 , HLBVH>;

} // namespace other

#endif // !OTHER_ENGINE_BVH_HPP
