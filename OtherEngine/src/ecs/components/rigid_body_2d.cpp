/**
 * \file ecs/components/rigid_body_2d.cpp
 **/
#include "ecs/components/rigid_body_2d.hpp"

#include "core/config_keys.hpp"

#include "ecs/entity.hpp"
#include "physics/physics_defines.hpp"

namespace other {

  void RigidBody2DSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    auto& rigid_body = entity->GetComponent<RigidBody2D>();

    SerializeComponentSection(stream , entity , "rigid-body-2D");
    
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

    SerializeValue(stream , "mass" , rigid_body.mass);
    SerializeValue(stream , "linear-drag" , rigid_body.linear_drag);
    SerializeValue(stream , "angular-drag" , rigid_body.angular_drag);
    SerializeValue(stream , "gravity-scale" , rigid_body.gravity_scale);
    SerializeValue(stream , "fixed-rotation" , rigid_body.fixed_rotation);
    SerializeValue(stream , "bullet" , rigid_body.bullet);
  }

  void RigidBody2DSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize a rigid-body 2D into null entity or scene!");
    std::string key_value = GetComponentSectionKey(entity->Name() , std::string{ kRigidBody2DValue });

    auto& rigid_body = entity->AddComponent<RigidBody2D>();
    
    auto body_type = scene_table.Get(key_value , kTypeValue);
    if (body_type.size() != 1) {
      OE_ERROR("Failed to deserialize rigidy body 2D into entity {}" , entity->Name());
      return;
    }

    std::string type = body_type[0];
    std::string uc_type;
    std::transform(type.begin() , type.end() , std::back_inserter(uc_type) , ::toupper);
  
    switch (FNV(uc_type)) {
      case kStaticValueHash:
        rigid_body.type = STATIC;
      break;
      case kKinematicValueHash:
        rigid_body.type = KINEMATIC;
      break;
      case kDynamicValueHash:
        rigid_body.type = DYNAMIC;
      break;
      default:
        OE_ERROR("Rigid body 2D section corrupt, cannot deserialize into entity {}" , entity->Name());
        entity->RemoveComponent<RigidBody2D>();
        return;
    }

    rigid_body.mass = scene_table.GetVal<float>(key_value , kMassValue).value_or(1.f);
    rigid_body.linear_drag = scene_table.GetVal<float>(key_value , kLinearDragValue).value_or(0.01f);
    rigid_body.angular_drag = scene_table.GetVal<float>(key_value , kAngularDragValue).value_or(0.05f);
    rigid_body.gravity_scale = scene_table.GetVal<float>(key_value , kGravityScaleValue).value_or(1.f);
    rigid_body.fixed_rotation = scene_table.GetVal<bool>(key_value , kFixedRotationValue).value_or(false);
    rigid_body.bullet = scene_table.GetVal<bool>(key_value , kBulletValue).value_or(false);
  }

} // namespace other
