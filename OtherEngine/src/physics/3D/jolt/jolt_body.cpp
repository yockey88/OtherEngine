/**
 * \file physics/3D/jolt/jolt_body.cpp
 **/
#include "physics/3D/jolt/jolt_body.hpp"

#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "ecs/components/rigid_body.hpp"
#include "ecs/entity.hpp"

#include "physics/physics_defines.hpp"

namespace other {

  JoltBody::JoltBody(JPH::BodyInterface& body_interface, Entity* entity)
      : body_interface(body_interface), entity(entity) {
    const RigidBody& body = entity->ReadComponent<RigidBody>();

    switch (body.type) {
      case PhysicsBodyType::STATIC: {
        // CreateStaticBody(body_interface);
        break;
      }
      case PhysicsBodyType::DYNAMIC:
      case PhysicsBodyType::KINEMATIC: {
        // CreateDynamicBody(body_interface);
        break;
      }
      default:
        OE_ASSERT(false, "Invalid body type : {}", body.type);
        break;
    }

    // m_OldMotionType = JoltUtils::ToJoltMotionType(rigidBodyComponent.BodyType);
  }

}  // namespace other