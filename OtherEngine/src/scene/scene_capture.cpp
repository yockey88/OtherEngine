/**
 * \file scene/scene_capture.hpp
 **/
#include "scene/scene_capture.hpp"

#include <map>

#include <entt/entity/fwd.hpp>

#include "core/logger.hpp"

#include "ecs/component.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/collider.hpp"
#include "ecs/components/collider_2d.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/rigid_body.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/serialization_data.hpp"
// #include "ecs/components/sprite_2d.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/entity.hpp"
#include "scene/scene.hpp"

namespace other {

  template <typename CT>
  static void CopyInto(entt::registry& dest, entt::registry& src, const std::map<UUID, entt::entity>& new_entts) {
    auto view = src.view<CT>();
    for (auto old_entt : view) {
      Tag& tag = src.get<Tag>(old_entt);
      entt::entity new_entt = new_entts.at(tag.id);

      CT& src_comp = src.get<CT>(old_entt);
      dest.emplace_or_replace<CT>(new_entt, src_comp);
    }
  }

  template <typename CT, typename Fn>
    requires requires(CT& comp, Fn fn) {
      { fn(comp, std::declval<entt::registry&>(), std::declval<entt::entity>()) } -> std::same_as<void>;
    }
  static void CopyIntoAndThen(entt::registry& dest, entt::registry& src, const std::map<UUID, entt::entity>& new_entts, Fn&& fn) {
    CopyInto<CT>(dest, src, new_entts);
    auto view = dest.view<CT>();

    for (auto new_entt : view) {
      CT& comp = dest.get<CT>(new_entt);
      std::forward<Fn>(fn)(comp, dest, new_entt);
    }
  }

  // static void CaptureSnapshot(Ref<SceneCapture>& output, entt::registry& input) {
  //   entt::snapshot{ input }
  //     .get<entt::entity>(output->registry)
  //     .get<Tag>(output->registry)
  //     .get<Transform>(output->registry)
  //     .get<Relationship>(output->registry)
  //     .get<SerializationData>(output->registry)
  //     .get<Script>(output->registry)
  //     .get<Mesh>(output->registry)
  //     .get<StaticMesh>(output->registry)
  //     .get<RigidBody>(output->registry)
  //     .get<Collider>(output->registry)
  //     .get<RigidBody2D>(output->registry)
  //     .get<Collider2D>(output->registry);
  // }

  // static void LoadSnapshot(entt::registry& dest, Ref<SceneCapture>& src) {
  //   entt::snapshot_loader{ dest }
  //     .get<entt::entity>(src->registry)
  //     .get<Tag>(src->registry)
  //     .get<Transform>(src->registry)
  //     .get<Relationship>(src->registry)
  //     .get<SerializationData>(src->registry)
  //     .get<Script>(src->registry)
  //     .get<Mesh>(src->registry)
  //     .get<StaticMesh>(src->registry)
  //     .get<RigidBody>(src->registry)
  //     .get<Collider>(src->registry)
  //     .get<RigidBody2D>(src->registry)
  //     .get<Collider2D>(src->registry);
  // }

  bool CaptureStack::PopCapture(Scene* scene) {
    OE_ASSERT(scene != nullptr, "Scene is null");
    OE_ASSERT(scene->SceneHandle() == scene_id, "Scene id mismatch");

    if (capture_stack.empty()) {
      return false;
    }

    if (scene == nullptr) {
      while (!capture_stack.empty()) {
        capture_stack.pop();
      }
      return true;
    }

    auto& registry = scene->Registry();
    const std::map<UUID, Entity*>& entities = scene->SceneEntities();
    std::map<UUID, entt::entity> ent_map{};
    for (const auto& [id, ent] : entities) {
      auto [itr, res] = ent_map.insert({ id, ent->Handle() });
      OE_ASSERT(res, "Failed to insert entity into capture map");
      OE_ASSERT(itr != ent_map.end(), "Failed to insert entity into capture map");
    }
    {
      Ref<SceneCapture>& capture = capture_stack.top();

      CopyInto<Transform>(registry, capture->registry, ent_map);
      CopyInto<Relationship>(registry, capture->registry, ent_map);
      CopyInto<SerializationData>(registry, capture->registry, ent_map);
      CopyInto<Script>(registry, capture->registry, ent_map);
      CopyInto<Mesh>(registry, capture->registry, ent_map);
      CopyInto<StaticMesh>(registry, capture->registry, ent_map);

      CopyIntoAndThen<RigidBody>(registry, capture->registry, ent_map, [](RigidBody& body, entt::registry& reg, entt::entity ent) {
        Transform& transform = reg.get<Transform>(ent);
        body.physics_body->SetTransform(transform);
      });
      CopyInto<Collider>(registry, capture->registry, ent_map);

      CopyIntoAndThen<RigidBody2D>(registry, capture->registry, ent_map, [](RigidBody2D& body, entt::registry& reg, entt::entity ent) {
        // Transform& transform = reg.get<Transform>(ent);
        // body.physics_body->SetTransform(transform);
      });
      CopyInto<Collider2D>(registry, capture->registry, ent_map);

      CopyInto<LightSource>(registry, capture->registry, ent_map);
      CopyInto<Camera>(registry, capture->registry, ent_map);
      // CopyInto<Sprite2D>(registry, capture->registry, ent_map);

      capture_stack.pop();
    }

    return true;
  }

  void CaptureStack::PushCapture(Scene* scene) {
    OE_ASSERT(scene != nullptr, "Scene is null");
    scene_id = scene->SceneHandle();

    Ref<SceneCapture> capture = NewRef<SceneCapture>();

    auto& registry = scene->Registry();
    const std::map<UUID, Entity*>& entities = scene->SceneEntities();
    registry.sort<Tag>([&](const Tag& lhs, const Tag& rhs) {
      auto lent = entities.find(lhs.id);
      auto rent = entities.find(rhs.id);
      OE_ASSERT(lent != entities.end(), "Entity not found in scene");
      OE_ASSERT(rent != entities.end(), "Entity not found in scene");
      return lent->second->GetUUID().Get() < rent->second->GetUUID().Get();
    });

    for (const auto& [id, ent] : entities) {
      auto [itr, res] = capture->entity_map.insert({ id, capture->registry.create() });
      OE_ASSERT(res, "Failed to insert entity into capture map");
      OE_ASSERT(itr != capture->entity_map.end(), "Failed to insert entity into capture map");
      capture->registry.emplace<Tag>(itr->second, ent->GetComponent<Tag>());
    }

    CopyInto<Transform>(capture->registry, registry, capture->entity_map);
    CopyInto<Relationship>(capture->registry, registry, capture->entity_map);
    CopyInto<SerializationData>(capture->registry, registry, capture->entity_map);
    CopyInto<Script>(capture->registry, registry, capture->entity_map);
    CopyInto<Mesh>(capture->registry, registry, capture->entity_map);
    CopyInto<StaticMesh>(capture->registry, registry, capture->entity_map);
    CopyInto<RigidBody>(capture->registry, registry, capture->entity_map);
    CopyInto<Collider>(capture->registry, registry, capture->entity_map);
    CopyInto<RigidBody2D>(capture->registry, registry, capture->entity_map);
    CopyInto<Collider2D>(capture->registry, registry, capture->entity_map);
    CopyInto<LightSource>(capture->registry, registry, capture->entity_map);
    CopyInto<Camera>(capture->registry, registry, capture->entity_map);
    // CopyInto<Sprite2D>(capture->registry, registry, capture->entity_map);

    capture_stack.push(capture);
  }

}  // namespace other
