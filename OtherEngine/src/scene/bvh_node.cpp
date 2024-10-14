/**
 * \file scene/bvh_node.cpp
 **/
#include "scene/bvh_node.hpp"

#include <algorithm>
#include <array>
#include <stack>

#include <glm/common.hpp>

#include "math/vecmath.hpp"

#include "scene/bvh.hpp"

namespace other {

  template <>
  void BvhNode<8>::Update() {}

  template <>
  void BvhNode<2>::Update() {
    if (IsLeaf()) {
      OE_ASSERT(entities.size() == 1, "Leaf node has invalid number of entites! {} [{} , {}]", entities.size(), tree_index, bbox);
      auto* entity = entities[0];
      auto& t = entity->ReadComponent<Transform>();

      /// scale each bounding box so the entity's max extent fits inside the bounding box
      glm::vec3 half_scale = vec3_div(t.scale, 2.f);
      glm::vec3 e_min = vec3_sub(t.position, half_scale);
      glm::vec3 e_max = vec3_sum(t.position, half_scale);
      float scale_factor = EpsilonQuotient(glm::sqrt(3.f) - 1, 2);
      glm::vec3 real_min = vec3_sub(e_min, scale_factor);
      glm::vec3 real_max = vec3_sum(e_max, scale_factor);
      bbox = BBox(real_min, real_max);
      global_position = bbox.Center();

      if (parent != nullptr) {
        BBox parent_bbox = parent->bbox;

        BvhChildIdx cousin = parent->partition_index == LEFT ? RIGHT : LEFT;
        BvhNode<2>* cousin_node = nullptr;

        if (parent->parent != nullptr && parent->parent->children[cousin] != nullptr) {
          cousin_node = parent->parent->children[cousin];
        }

        if (cousin_node != nullptr && parent_bbox.SurfaceArea() > cousin_node->bbox.SurfaceArea()) {
          built = false;
        }
      }

      return;
    }
    /// else if its not a leaf and we dont contain our children
    else if (children[LEFT] != nullptr && children[RIGHT] != nullptr && (!Contains(children[LEFT]->bbox.min) || !Contains(children[RIGHT]->bbox.max))) {
      bbox = BBox::Union(children[LEFT]->bbox, children[RIGHT]->bbox);
      global_position = bbox.Center();
    } else if (children[LEFT] != nullptr) {
      bbox = BBox::Union(bbox, children[LEFT]->bbox);
    } else if (children[RIGHT] != nullptr) {
      bbox = BBox::Union(bbox, children[RIGHT]->bbox);
    }

    for (auto* child : children) {
      if (child == nullptr) {
        continue;
      }

      child->Update();
      if (built) {
        built = child->built;
      }
    }
  }

  template <>
  void BvhNode<8>::ExpandToInclude(const glm::vec3& point) {
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
    if (vec3_lt(child->Min(), Min())) {
      min = child->Min();
    }
    if (vec3_lt(max, child->Max())) {
      max = child->Max();
    }

    bbox = BBox(min, max);
  }

  template <>
  void BvhNode<2>::ExpandToInclude(const glm::vec3& point) {
    OE_ASSERT(false, "ExpandToInclude not implemented for BvhNode<2 , HLBVH>!");
  }

  template <>
  void BvhNode<8>::InsertEntity(Entity* entity, const glm::vec3& position, uint8_t location) {
    entities.push_back(entity);

    if (IsLeaf()) {
      return;
    }

    size_t partition_idx = kLocationIndex.at(location);
    auto* child = children[partition_idx];
    if (child == nullptr) {
      return;
    }

    child->InsertEntity(entity, position, location);
  }

  template <>
  bool BvhNode<8>::NeedsRebuild(BvhNode<8>* space, const std::vector<Entity*>& entities) {
    OE_ASSERT(false, "NeedsRebuild not implemented for BvhNode<8 , OCTREE>!");
  }

  template <>
  bool BvhNode<2>::NeedsRebuild(BvhNode<2>* space, const std::vector<Entity*>& entities) {
    if (space->IsLeaf()) {
      return false;
    }

    return false;
  }

  namespace {

    int64_t GetCenterIdx(const std::vector<BvhNode<2>*>& nodes, const glm::vec3& global_position) {
      int64_t idx = -1;
      if (nodes.size() % 2 == 1) {
        idx = nodes.size() / 2;
      } else {
        size_t test1 = nodes.size() / 2;
        size_t test2 = (nodes.size() / 2) - 1;

        glm::vec3 center1 = nodes[test1]->bbox.Center();
        glm::vec3 center2 = nodes[test2]->bbox.Center();

        float dist_from_origin1 = glm::length(vec3_diff(center1, global_position));
        float dist_from_origin2 = glm::length(vec3_diff(center2, global_position));

        idx = dist_from_origin1 < dist_from_origin2 ? test1 : test2;
      }
      OE_ASSERT(idx >= 0, "Invalid index : {}", idx);
      return idx;
    }

    std::pair<std::vector<BvhNode<2>*>, std::vector<BvhNode<2>*>> SplitOnIndex(int64_t idx, const std::vector<BvhNode<2>*>& nodes) {
      OE_ASSERT(idx >= 0, "Invalid index : {}", idx);

      size_t left_end = idx;
      size_t right_begin = idx + 1;
      auto left_group = nodes | std::views::take(left_end) | std::ranges::to<std::vector<BvhNode<2>*>>();
      auto right_group = nodes | std::views::drop(right_begin) | std::ranges::to<std::vector<BvhNode<2>*>>();

      return {
        left_group,
        right_group,
      };
    }

    BBox BoundingBoxFromEntity(Entity* entity) {
      OE_ASSERT(entity != nullptr, "Entity is null!");
      auto& t = entity->ReadComponent<Transform>();

      glm::vec3 half_scale = vec3_div(t.scale, 2.f);
      glm::vec3 e_min = vec3_sub(t.position, half_scale);
      glm::vec3 e_max = vec3_sum(t.position, half_scale);
      float scale_factor = EpsilonQuotient(glm::sqrt(3.f) - 1, 2);
      glm::vec3 real_min = vec3_sub(e_min, scale_factor);
      glm::vec3 real_max = vec3_sum(e_max, scale_factor);

      return BBox(real_min, real_max);
    }

  }  // namespace

  template <>
  BvhNode<2>* BvhNode<2>::TreeFromSortedList(BvhNode<2>* space, const std::vector<BvhNode<2>*>& nodes) {
    OE_ASSERT(space != nullptr, "Space is null!");
    if (nodes.empty()) {
      return nullptr;
    }

    int64_t idx = GetCenterIdx(nodes, space->global_position);
    BvhNode<2>* center_node = nodes[idx];
    OE_ASSERT(center_node != nullptr, "Invalid center node!");

    auto [left_group, right_group] = SplitOnIndex(idx, nodes);
    auto* lchild = TreeFromSortedList(center_node, left_group);
    auto* rchild = TreeFromSortedList(center_node, right_group);
    if (lchild != nullptr) {
      lchild->partition_index = LEFT;
    }
    if (rchild != nullptr) {
      rchild->partition_index = RIGHT;
    }

    if (lchild == nullptr && rchild == nullptr) {
      return center_node;
    }
    /// if both children are not null then we group them into a node with a bounding box
    ///   that is the union of the children bounding boxes
    else if (lchild != nullptr && rchild != nullptr) {
      BvhNode<2>* parent = nullptr;
      {
        auto [tidx, new_node] = space->nodes->EmplaceBackNoLock();
        new_node.tree = space->tree;
        new_node.nodes = space->nodes;
        new_node.tree_index = tidx;
        new_node.partition_index = 0;
        new_node.partition_location = 0b000;
        new_node.parent = center_node;
        parent = &(*space->nodes)[tidx];
      }

      /// find the pairing (center and left/center and right) that creates the smallest surface area bounding box
      ///   and make 'parent' the parent of those two with that bounding box making parent the left child of 'space'
      ///   and the other child the right child of 'space'

      BBox lchild_box = BBox::Union(center_node->bbox, lchild->bbox);
      BBox rchild_box = BBox::Union(center_node->bbox, rchild->bbox);

      float lchild_surface_area = lchild_box.SurfaceArea();
      float rchild_surface_area = rchild_box.SurfaceArea();
      BBox smaller = lchild_surface_area < rchild_surface_area ? lchild_box : rchild_box;
      center_node->partition_index = lchild_surface_area < rchild_surface_area ?
        RIGHT :
        LEFT;

      parent->bbox = smaller;
      parent->global_position = smaller.Center();

      BvhNode<2>* space_lchild = nullptr;
      BvhNode<2>* space_rchild = nullptr;
      if (lchild_surface_area < rchild_surface_area) {
        lchild->parent = parent;
        center_node->parent = parent;

        parent->children[LEFT] = lchild;
        parent->children[RIGHT] = center_node;
        for (const auto& [le, re] : std::views::zip(lchild->entities, center_node->entities)) {
          parent->entities.push_back(le);
          parent->entities.push_back(re);
        }

        parent->partition_index = LEFT;
        space_lchild = parent;
        space_rchild = rchild;
      } else {
        parent->children[LEFT] = center_node;
        parent->children[RIGHT] = rchild;
        for (const auto& [le, re] : std::views::zip(center_node->entities, rchild->entities)) {
          parent->entities.push_back(le);
          parent->entities.push_back(re);
        }

        parent->partition_index = RIGHT;
        space_lchild = lchild;
        space_rchild = parent;
      }
      /// sanity check
      OE_ASSERT(lchild != nullptr, "Left child is null!");
      OE_ASSERT(rchild != nullptr, "Right child is null!");

      space_lchild->parent = space;
      space_rchild->parent = space;

      space->children[LEFT] = space_lchild;
      space->children[RIGHT] = space_rchild;
      space->bbox = BBox::Union(lchild->bbox, rchild->bbox);
      for (const auto& [le, re] : std::views::zip(lchild->entities, rchild->entities)) {
        if (std::ranges::find(space->entities, le) == space->entities.end()) {
          space->entities.push_back(le);
        }
        if (std::ranges::find(space->entities, re) == space->entities.end()) {
          space->entities.push_back(re);
        }
      }

      return space;
    }
    /// no need to check which isnt null cause we keep the valid child and the null child index is already null
    ///   so it remains the same
    else {
      center_node->children[LEFT] = lchild;
      center_node->children[RIGHT] = rchild;
      if (lchild != nullptr) {
        lchild->parent = center_node;
        std::ranges::copy(lchild->entities, std::back_inserter(center_node->entities));
      }

      if (rchild != nullptr) {
        rchild->parent = center_node;
        std::ranges::copy(rchild->entities, std::back_inserter(center_node->entities));
      }
    }

    return center_node;
  }

  template <>
  BvhNode<8>* BvhNode<8>::RebuildTree(BvhNode<8>* space, std::vector<Entity*>& entities) {
    OE_ASSERT(false, "RebuildTree not implemented for BvhNode<8>!");
  }

  template <>
  BvhNode<2>* BvhNode<2>::RebuildTree(BvhNode<2>* space, std::vector<Entity*>& entities) {
    OE_ASSERT(space != nullptr, "Space is null!");

    /// if entities is empty space is a leaf node with no size, if it is one then it is a leaf node with the bounding box of the entity
    if (entities.empty() || entities.size() == 1) {
      space->built = true;
      if (entities.size() == 1) {
        std::ranges::copy(entities, std::back_inserter(space->entities));
        space->bbox = BoundingBoxFromEntity(entities[0]);
        space->global_position = space->bbox.Center();
      }
      return space;
    }

    std::ranges::sort(entities, [](const Entity* e1, const Entity* e2) -> bool {
      auto& t1 = e1->ReadComponent<Transform>();
      auto& t2 = e2->ReadComponent<Transform>();
      return vec3_lt(t1.position, t2.position);
    });

    auto create_node = [&](Entity* ent) -> BvhNode<2>* {
      OE_ASSERT(ent != nullptr, "Entity is null!");
      auto [tidx, node] = space->nodes->EmplaceBackNoLock();
      node.tree = space->tree;
      node.nodes = space->nodes;

      /// scale each bounding box so the entity's max extent fits inside the bounding box
      node.bbox = BoundingBoxFromEntity(ent);
      node.global_position = node.bbox.Center();

      node.tree_index = tidx;
      node.partition_index = 0;
      node.partition_location = 0b000;
      node.parent = nullptr;
      node.entities.push_back(ent);
      std::ranges::fill(node.children, nullptr);

      return &(*space->nodes)[tidx];
    };

    std::stack<BvhNode<2>*> pear_stack{};
    std::vector<BvhNode<2>*> nodes =
      entities |
      std::views::transform(create_node) |
      std::ranges::to<std::vector<BvhNode<2>*>>();

    OE_ASSERT(nodes.size() >= 2 && nodes.size() == entities.size(), "Invalid number of nodes : {} != {}", nodes.size(), entities.size());
    if (nodes.size() == 2) {
      auto* lchild = nodes[0];
      auto* rchild = nodes[1];
      OE_ASSERT(lchild != nullptr, "Left child is null!");
      OE_ASSERT(rchild != nullptr, "Right child is null!");

      space->bbox = BBox::Union(lchild->bbox, rchild->bbox);
      space->children[LEFT] = lchild;
      space->children[RIGHT] = rchild;

      for (const auto& [le, re] : std::views::zip(lchild->entities, rchild->entities)) {
        space->entities.push_back(le);
        space->entities.push_back(re);
      }
    } else {
      space = TreeFromSortedList(space, nodes);
      if (space->children[LEFT] != nullptr && space->children[RIGHT] != nullptr) {
        space->bbox = BBox::Union(space->children[LEFT]->bbox, space->children[RIGHT]->bbox);
      }
    }

    space->nodes->ForEach([](auto& node) mutable { node.built = true; });
    return space;
  }

  template <>
  void BvhNode<2>::InsertEntity(Entity* entity, const glm::vec3& position, uint8_t location) {
    OE_ASSERT(entity != nullptr, "Entity is null!");
    OE_ASSERT(tree != nullptr, "Tree is null!");
    entities.push_back(entity);
    tree->entities.push_back(entity);
  }

}  // namespace other