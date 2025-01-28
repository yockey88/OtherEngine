/**
 * \file ecs/components/physics_component.cpp
 **/
#include "ecs/components/physics_component.hpp"

#include "core/config_keys.hpp"

#include "ecs/components/transform.hpp"
#include "ecs/entity.hpp"
#include "scene/scene.hpp"

namespace other {

  void RigidBodySerializer::Serialize(std::ostream& stream, Entity* entity, const Ref<Scene>& scene) const {
    auto& rigid_body = entity->GetComponent<RigidBody>();

    SerializeComponentSection(stream, entity, "rigid-body");

    stream << "type = ";
    switch (rigid_body.type) {
      case PhysicsBodyType::STATIC:
        stream << "\"static\"\n";
        break;
      case PhysicsBodyType::KINEMATIC:
        stream << "\"kinematic\"\n";
        break;
      case PhysicsBodyType::DYNAMIC:
        stream << "\"dynamic\"\n";
        break;
      default:
        break;
    }

    SerializeValue(stream, "mass", rigid_body.mass);
    SerializeValue(stream, "layer-id", rigid_body.layer_id);
    SerializeValue(stream, "linear-drag", rigid_body.linear_drag);
    SerializeValue(stream, "angular-drag", rigid_body.angular_drag);
    SerializeValue(stream, "disable-gravity", rigid_body.disable_gravity);
    SerializeValue(stream, "is-trigger", rigid_body.is_trigger);

    stream << "collision-type = ";
    switch (rigid_body.collision_type) {
      case CollisionDetectionType::DISCRETE_COLLISION:
        stream << "\"discrete\"";
        break;
      case CollisionDetectionType::CONTINUOUS_COLLISION:
        stream << "\"continuous\"";
        break;
      default:
        break;
    }

    SerializeVec3(stream, "initial-linear-velocity", rigid_body.initial_linear_velocity);
    SerializeVec3(stream, "initial-angular-velocity", rigid_body.initial_angular_velocity);

    SerializeValue(stream, "max-linear-velocity", rigid_body.max_linear_velocity);
    SerializeValue(stream, "max-angular-velocity", rigid_body.max_angular_velocity);
  }

  void RigidBodySerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr, "Attempting to deserialize a rigid-body into null entity or scene!");

    auto& rigid_body = entity->AddComponent<RigidBody>();
    std::string key_value = "";
    if (force_deserialize) {
      key_value = GetComponentSectionKey(entity->Name(), std::string{ kPhysicsObjectValue });
    } else {
      key_value = GetComponentSectionKey(entity->Name(), std::string{ kRigidBodyValue });
    }
    if (!scene_table.SectionExists(key_value)) {
      OE_ERROR("Rigid body section [{}] not found for entity {}", key_value, entity->Name());
      entity->RemoveComponent<RigidBody>();
      return;
    }

    std::string body_type = "";
    auto body_type_opt = scene_table.GetVal<std::string>(key_value, kTypeValue);
    if (force_deserialize && !body_type_opt.has_value()) {
      body_type = "dynamic";
    } else if (body_type_opt.has_value()) {
      body_type = body_type_opt.value();
    } else {
      OE_ERROR("Rigid body type not found for entity {}", entity->Name());
      entity->RemoveComponent<RigidBody>();
      return;
    }

    OE_DEBUG("Rigid body type : {}", body_type);
    switch (FNV(body_type)) {
      case FNV("static"):
        rigid_body.type = STATIC;
        break;
      case FNV("kinematic"):
        rigid_body.type = KINEMATIC;
        break;
      case FNV("dynamic"):
        rigid_body.type = DYNAMIC;
        break;
      default:
        OE_ERROR("Rigid body type section corrupt, cannot deserialize into entity {}", entity->Name());
        entity->RemoveComponent<RigidBody>();
        return;
    }

    rigid_body.mass = scene_table.GetVal<float>(key_value, kMassValue, false).value_or(1.f);
    rigid_body.linear_drag = scene_table.GetVal<float>(key_value, kLinearDragValue, false).value_or(0.01f);
    rigid_body.angular_drag = scene_table.GetVal<float>(key_value, kAngularDragValue, false).value_or(0.05f);
    rigid_body.disable_gravity = scene_table.GetVal<bool>(key_value, kDisableGravityValue, false).value_or(false);
    rigid_body.is_trigger = scene_table.GetVal<bool>(key_value, kIsTriggerValue, false).value_or(false);

    auto collision_type = scene_table.GetVal<std::string>(key_value, kSimulationTypeValue).value_or("discrete");
    switch (FNV(collision_type)) {
      case kDiscreteValueHash:
        rigid_body.collision_type = DISCRETE_COLLISION;
        break;
      case kContinuousValueHash:
        rigid_body.collision_type = CONTINUOUS_COLLISION;
        break;
      default:
        return;
    }

    const auto& init_lin_vel = scene_table.Get(key_value, kInitialLinearVelocityValue);
    const auto& init_ang_vel = scene_table.Get(key_value, kInitialAngularVelocityValue);

    if (init_lin_vel.size() > 0) {
      DeserializeVec3(init_lin_vel, rigid_body.initial_linear_velocity);
    }

    if (init_ang_vel.size() > 0) {
      DeserializeVec3(init_ang_vel, rigid_body.initial_angular_velocity);
    }

    rigid_body.max_linear_velocity = scene_table.GetVal<float>(key_value, kMaxLinearVelocityValue, false).value_or(500.f);
    rigid_body.max_angular_velocity = scene_table.GetVal<float>(key_value, kMaxAngularVelocityValue, false).value_or(50.f);
  }

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
      case PhysicsShape::Shape::CONCAVE_MESH:
        stream << "\"concave-mesh\"\n";
        break;
      case PhysicsShape::Shape::CONVEX_MESH:
        stream << "\"convex-mesh\"\n";
        break;
      default:
        OE_ERROR("Collider shape not found for entity {}", entity->Name());
        return;
    }
  }

  void ColliderSerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr, "Attempting to deserialize a rigid-body into null entity or scene!");

    std::string key_value = "";
    if (force_deserialize) {
      key_value = GetComponentSectionKey(entity->Name(), std::string{ kPhysicsObjectValue });
    } else {
      key_value = GetComponentSectionKey(entity->Name(), std::string{ kColliderValue });
    }
    if (!scene_table.SectionExists(key_value)) {
      OE_ERROR("Collider section [{}] not found for entity {}", key_value, entity->Name());
      return;
    }

    std::string shape = "";
    auto shape_opt = scene_table.GetVal<std::string>(key_value, kShapeValue);
    if (force_deserialize && !shape_opt.has_value()) {
      shape = "box";
    } else if (shape_opt.has_value()) {
      shape = shape_opt.value();
    } else {
      OE_ERROR("Collider shape not found for entity {}", entity->Name());
      return;
    }

    switch (FNV(shape)) {
      case FNV("box"):
        entity->AddComponent<Collider>(PhysicsShape::Shape::BOX);
        break;
      case FNV("sphere"):
        entity->AddComponent<Collider>(PhysicsShape::Shape::SPHERE);
        break;
      case FNV("capsule"):
        entity->AddComponent<Collider>(PhysicsShape::Shape::CAPSULE);
        break;
      case FNV("convex-mesh"):
        entity->AddComponent<Collider>(PhysicsShape::Shape::CONVEX_MESH);
        break;
      case FNV("concave-mesh"):
        entity->AddComponent<Collider>(PhysicsShape::Shape::CONCAVE_MESH);
        break;
      //   break;
      default:
        OE_ERROR("Collider shape section corrupt, cannot deserialize into entity {}", entity->Name());
        entity->RemoveComponent<Collider>();
        return;
    }
  }

  void PhysicsObjectSerializer::Serialize(std::ostream& stream, Entity* entity, const Ref<Scene>& scene) const {
    // SerializeComponentSection(stream, entity, "rigid-body");
    // SerializeComponentSection(stream, entity, "physics-object");
    // SerializeValue(stream, "body", physics_object.body->GetEntityID());
    // SerializeValue(stream, "shape", physics_object.shape->GetEntityID());
  }

  void PhysicsObjectSerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    ColliderSerializer c_serializer{};
    c_serializer.force_deserialize = true;
    c_serializer.Deserialize(entity, scene_table, scene);

    RigidBodySerializer rb_serializer{};
    rb_serializer.force_deserialize = true;
    rb_serializer.Deserialize(entity, scene_table, scene);

    [[maybe_unused]] auto& physics_object = entity->AddComponent<PhysicsObject>();

    RigidBody& body = entity->GetComponent<RigidBody>();
    Collider& collider = entity->GetComponent<Collider>();
    Transform& transform = entity->GetComponent<Transform>();

    collider.shape->SetTransform(transform);
    body.physics_body->SetTransform(transform);
  }

}  // namespace other