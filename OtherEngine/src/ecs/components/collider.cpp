
/**
 * \file ecs/components/collider.cpp
 **/
#include "ecs/components/collider.hpp"

#include "core/config_keys.hpp"
#include "core/defines.hpp"
#include "core/logger.hpp"

#include "ecs/components/rigid_body.hpp"
#include "ecs/entity.hpp"
#include "scene/scene.hpp"

namespace other {

  void ColliderSerializer::Serialize(std::ostream& stream, Entity* entity, const Ref<Scene>& scene) const {
    auto& collider = entity->GetComponent<Collider>();

    SerializeComponentSection(stream, entity, "collider");
    stream << "shape = ";

    switch (collider.shape->ShapeType()) {
      case PhysicsShape::Shape::BOX:
        stream << "box\n";
        break;
      case PhysicsShape::Shape::SPHERE:
        stream << "sphere\n";
        break;
      case PhysicsShape::Shape::CAPSULE:
        stream << "capsule\n";
        break;
      // case PhysicsShape::Shape::CONCAVE_MESH:
      //   stream << "\"concave-mesh\"\n";
      //   break;
      // case PhysicsShape::Shape::CONVEX_MESH:
      //   stream << "\"convex-mesh\"\n";
      //   break;
      default:
        OE_ERROR("Collider shape not found for entity {}", entity->Name());
        return;
    }
  }

  void ColliderSerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr, "Attempting to deserialize a rigid-body into null entity or scene!");

    auto& collider = entity->AddComponent<Collider>();
    std::string key_value = GetComponentSectionKey(entity->Name(), std::string{ kColliderValue });

    auto shape = scene_table.GetVal<std::string>(key_value, kShapeValue);
    if (!shape.has_value()) {
      OE_ERROR("Collider shape not found for entity {}", entity->Name());
      entity->RemoveComponent<Collider>();
      return;
    }

    Transform& transform = entity->GetComponent<Transform>();

    switch (FNV(shape.value())) {
      case FNV("box"):
        collider.shape = scene->GetPhysicsWorld()->CreateBoxShape(transform.scale / 2.f);
        break;
      case FNV("sphere"):
        collider.shape = scene->GetPhysicsWorld()->CreateSphereShape(transform.scale.x / 2.f);
        break;
      case FNV("capsule"):
        collider.shape = scene->GetPhysicsWorld()->CreateCapsuleShape(transform.scale.x / 2.f, transform.scale.y);
        break;
      // case FNV("concave-mesh"):
      //   collider.shape = scene->GetPhysicsWorld()->CreateConcaveMeshShape();
      //   break;
      // case FNV("convex-mesh"):
      //   collider.shape = scene->GetPhysicsWorld()->CreateConvexMeshShape();
      //   break;
      default:
        OE_ERROR("Collider shape section corrupt, cannot deserialize into entity {}", entity->Name());
        entity->RemoveComponent<Collider>();
        return;
    }
    OE_ASSERT(collider.shape != nullptr, "Failed to create collider shape for entity {}", entity->Name());
  }

}  // namespace other
