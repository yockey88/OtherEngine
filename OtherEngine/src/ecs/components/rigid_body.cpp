
/**
 * \file ecs/components/rigid_body.cpp
 **/
#include "ecs/components/rigid_body.hpp"

#include "core/config_keys.hpp"
#include "core/logger.hpp"

#include "ecs/entity.hpp"
#include "scene/scene.hpp"

#include "physics/physics_defines.hpp"

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
    std::string key_value = GetComponentSectionKey(entity->Name(), std::string{ kRigidBodyValue });

    auto body_type = scene_table.GetVal<std::string>(key_value, kTypeValue);
    if (!body_type.has_value()) {
      OE_ERROR("Failed to deserialize rigidy body into entity {}", entity->Name());
      return;
    }

    OE_DEBUG("Rigid body type : {}", *body_type);
    switch (FNV(*body_type)) {
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

    auto init_lin_vel = scene_table.Get(key_value, kInitialLinearVelocityValue);
    auto init_ang_vel = scene_table.Get(key_value, kInitialAngularVelocityValue);

    if (init_lin_vel.size() > 0) {
      DeserializeVec3(init_lin_vel, rigid_body.initial_linear_velocity);
    }

    if (init_ang_vel.size() > 0) {
      DeserializeVec3(init_ang_vel, rigid_body.initial_angular_velocity);
    }

    rigid_body.max_linear_velocity = scene_table.GetVal<float>(key_value, kMaxLinearVelocityValue, false).value_or(500.f);
    rigid_body.max_angular_velocity = scene_table.GetVal<float>(key_value, kMaxAngularVelocityValue, false).value_or(50.f);
  }

}  // namespace other
