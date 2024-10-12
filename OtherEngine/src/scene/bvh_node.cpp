/**
 * \file scene/bvh_node.cpp
 **/
#include "scene/bvh_node.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <stack>
#include <queue>

#include "math/vecmath.hpp"
#include "scene/bvh.hpp"

namespace other {

  template <>
  void BvhNode<8 , OCTREE>::ExpandToInclude(const glm::vec3& point) {
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

    bbox = BBox(min , max);
  }

  template <> 
  void BvhNode<2 , HLBVH>::ExpandToInclude(const glm::vec3& point) {
    OE_ASSERT(false , "ExpandToInclude not implemented for BvhNode<2 , HLBVH>!");
  }

  template <>
  void BvhNode<8 , OCTREE>::InsertEntity(Entity* entity , const glm::vec3& position , uint8_t location) {
    entities.push_back(entity);

    if (IsLeaf()) {
      return;
    }

    size_t partition_idx = kLocationIndex.at(location);
    auto* child = children[partition_idx];
    if (child == nullptr) {
      return;
    }

    child->InsertEntity(entity , position , location);
  }
  
  template <>
  bool BvhNode<2 , HLBVH>::NeedsRebuild(BvhNode<2 , HLBVH>* space , const std::vector<Entity*>& entities) {
    return false;
    
    // if (space->IsLeaf()) { 
    //   return false;
    // }

    // for (const auto& e : entities) {
    //   if (!space->Contains(e->ReadComponent<Transform>().position)) {
    //     return true;
    //   }
    // }

    // bool left_rebuild = NeedsRebuild(space->children[LEFT] , entities);
    // bool right_rebuild = NeedsRebuild(space->children[RIGHT] , entities);

    // return left_rebuild || right_rebuild;
  }

  template <>
  BvhNode<8 , OCTREE>* BvhNode<8 , OCTREE>::TreeFromSortedList(BvhNode<2 , HLBVH>* space , const std::vector<BvhNode<8 , OCTREE>*>& nodes) {
    OE_ASSERT(false , "TreeFromSortedList not implemented for BvhNode<8 , OCTREE>!");
  }

  template <>
  BvhNode<2 , HLBVH>* BvhNode<2 , HLBVH>::TreeFromSortedList(BvhNode<2 , HLBVH>* space , const std::vector<BvhNode<2 , HLBVH>*>& nodes) {
    OE_ASSERT(space != nullptr , "Space is null!");
    if (nodes.empty()) {
      return nullptr;
    }

    BvhNode<2 , HLBVH>* center_node = nullptr;

    int64_t idx = -1;

    if (nodes.size() % 2 == 1) {
      idx = nodes.size() / 2;
    } else {
      size_t test1 = nodes.size() / 2;
      size_t test2 = (nodes.size() / 2) - 1;
      
      glm::vec3 center1 = nodes[test1]->bbox.Center();
      glm::vec3 center2 = nodes[test2]->bbox.Center();

      float dist_from_origin1 = glm::length(vec3_diff(center1 , space->global_position));
      float dist_from_origin2 = glm::length(vec3_diff(center2 , space->global_position));

      idx = dist_from_origin1 < dist_from_origin2 ? test1 : test2;
    }
    OE_ASSERT(idx >= 0 , "Invalid index : {}" , idx);
    
    center_node = nodes[idx]; 
    OE_ASSERT(center_node != nullptr , "Invalid center node!");
    
    size_t left_end = idx;
    size_t right_begin = idx + 1;
    auto left_group = nodes | std::views::take(left_end) | std::ranges::to<std::vector<BvhNode<2 , HLBVH>*>>();
    auto right_group = nodes | std::views::drop(right_begin) | std::ranges::to<std::vector<BvhNode<2 , HLBVH>*>>();

    auto* lchild = TreeFromSortedList(center_node , left_group);
    auto* rchild = TreeFromSortedList(center_node , right_group);

    if (lchild == nullptr && rchild == nullptr) {
      return center_node;
    }
    /// if both children are not null then we group them into a node with a bounding box
    ///   that is the union of the children bounding boxes
    else if (lchild != nullptr && rchild != nullptr) {
      BvhNode<2 , HLBVH>* parent = nullptr;
      {
        auto [tidx , new_node] = space->nodes->EmplaceBackNoLock();
        new_node.tree = space->tree;
        new_node.nodes = space->nodes;
        // new_node.bbox = BBox::Union(lchild->bbox , rchild->bbox);
        // new_node.global_position = new_node.bbox.Center();
        new_node.tree_index = tidx;
        new_node.partition_index = 0;
        new_node.partition_location = 0b000;
        new_node.parent = center_node;
        parent = &(*space->nodes)[tidx];
      }

      /// find the pairing (center and left/center and right) that creates the smallest surface area bounding box 
      ///   and make 'parent' the parent of those two with that bounding box making parent the left child of 'space' 
      ///   and the other child the right child of 'space'

      BBox lchild_box = BBox::Union(center_node->bbox , lchild->bbox);
      BBox rchild_box = BBox::Union(center_node->bbox , rchild->bbox);

      float lchild_surface_area = lchild_box.SurfaceArea();
      float rchild_surface_area = rchild_box.SurfaceArea();
      BBox smaller = lchild_surface_area < rchild_surface_area ? lchild_box : rchild_box;
      parent->bbox = smaller;
      parent->global_position = smaller.Center();

      BvhNode<2 , HLBVH>* space_lchild = nullptr;
      BvhNode<2 , HLBVH>* space_rchild = nullptr;
      if (lchild_surface_area < rchild_surface_area) {
        parent->children[LEFT] = lchild;
        parent->children[RIGHT] = center_node;
        for (const auto& e : lchild->entities) {
          parent->entities.push_back(e);
        }
        for (const auto& e : center_node->entities) {
          parent->entities.push_back(e);
        }
  
        space_lchild = parent;
        space_rchild = rchild;
      } else {
        parent->children[LEFT] = center_node;
        parent->children[RIGHT] = rchild;
        for (const auto& e : center_node->entities) {
          parent->entities.push_back(e);
        }
        for (const auto& e : rchild->entities) {
          parent->entities.push_back(e);
        }

        space_lchild = lchild;
        space_rchild = parent;
      }
      /// sanity check
      OE_ASSERT(lchild != nullptr , "Left child is null!");
      OE_ASSERT(rchild != nullptr , "Right child is null!");

      space->children[LEFT] = space_lchild;
      space->children[RIGHT] = space_rchild;
      space->bbox = BBox::Union(lchild->bbox , rchild->bbox);
      for (const auto& e : space_lchild->entities) {
        space->entities.push_back(e);
      }
      for (const auto& e : space_rchild->entities) {
        space->entities.push_back(e);
      }

      return space;
    } 
    /// no need to check which isnt null cause we keep the valid child and the null child index is already null
    ///   so it remains the same
    else {
      center_node->children[LEFT] = lchild;
      center_node->children[RIGHT] = rchild;
    }

    return center_node;
  }

  template <>
  BvhNode<2 , HLBVH>* BvhNode<2 , HLBVH>::RebuildTree(BvhNode<2 , HLBVH>* space , std::vector<Entity*>& entities) {
    OE_ASSERT(space != nullptr , "Space is null!");
    bool needs_rebuild = NeedsRebuild(space , entities);
    if (space->built) {
      return space;
    }

    if (entities.empty()) {
      return space;
    } else if (entities.size() == 1) {
      std::ranges::copy(entities , std::back_inserter(space->entities));
      return space;
    }

    std::ranges::sort(entities , [](const Entity* e1 , const Entity* e2) -> bool {
      auto& t1 = e1->ReadComponent<Transform>();
      auto& t2 = e2->ReadComponent<Transform>();
      return vec3_lt(t1.position , t2.position);
    });

    std::stack<BvhNode<2 , HLBVH>*> pear_stack{};
    std::vector<BvhNode<2 , HLBVH>*> nodes = entities |
      /// get the entity bounding boxes and their min max values
      std::views::transform([&](Entity* entity) -> BvhNode<2 , HLBVH>* {
        auto& t = entity->ReadComponent<Transform>();

        glm::vec3 half_scale = vec3_div(t.scale , 2.f);
        glm::vec3 e_min = vec3_sub(t.position, half_scale);
        glm::vec3 e_max = vec3_sum(t.position, half_scale);

        auto [tidx , node] = space->nodes->EmplaceBackNoLock();
        node.tree = space->tree;
        node.nodes = space->nodes;
        node.bbox = BBox(e_min , e_max);
        node.global_position = node.bbox.Center();
        node.tree_index = tidx;
        node.partition_index = 0;
        node.partition_location = 0b000;
        node.parent = nullptr;
        node.entities.push_back(entity);
        std::ranges::fill(node.children , nullptr);

        return &(*space->nodes)[tidx];
      }) |
      std::ranges::to<std::vector<BvhNode<2 , HLBVH>*>>();

    OE_ASSERT(nodes.size() >= 2 && nodes.size() == entities.size() , "Invalid number of nodes : {} != {}" , nodes.size() , entities.size());
    if (nodes.size() == 2) {
      auto* lchild = nodes[0];
      auto* rchild = nodes[1];
      OE_ASSERT(lchild != nullptr , "Left child is null!");
      OE_ASSERT(rchild != nullptr , "Right child is null!");

      space->bbox = BBox::Union(lchild->bbox , rchild->bbox);
      space->children[LEFT] = lchild;
      space->children[RIGHT] = rchild;

      for (const auto& [le , re] : std::views::zip(lchild->entities , rchild->entities)) {
        space->entities.push_back(le);
        space->entities.push_back(re);
      }
    } 
    /// > 2
    else {
      space = TreeFromSortedList(space , nodes);
    }

    space->built = true;
    return space;
  }

  template <> 
  void BvhNode<2 , HLBVH>::InsertEntity(Entity* entity , const glm::vec3& position , uint8_t location) {
    OE_ASSERT(entity != nullptr , "Entity is null!");
    entities.push_back(entity);
  }

} // namespace other