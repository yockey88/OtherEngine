/**
 * \file physics/3D/jolt/jolt_body.cpp
 **/
#include "physics/3D/jolt/jolt_body.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "ecs/components/physics_component.hpp"
#include "ecs/entity.hpp"

#include "physics/physics_defines.hpp"

namespace other {

  JoltBody::JoltBody(JPH::BodyInterface& body_interface, JPH::Body* body)
      : body_interface(body_interface), body(body) {
    body_id = body->GetID();

    if (body_id.IsInvalid()) {
      OE_ERROR("[Jolt] > Invalid body ID");
      return;
    }
    body_interface.SetObjectLayer(body_id, JPH::ObjectLayer(1));
    body_interface.AddBody(body_id, JPH::EActivation::Activate);
  }

  void JoltBody::OnSetNativeBody(void* body) {
    if (body == nullptr) {
      body = nullptr;
    } else {
      this->body = static_cast<JPH::Body*>(body);
    }
  }

  void JoltBody::SetTransform(const Transform& transform) {
    body_interface.SetPosition(body_id, JPH::RVec3Arg(transform.position.x, transform.position.y, transform.position.z), JPH::EActivation::Activate);
  }

  Transform JoltBody::GetTransform() const {
    JPH::RVec3 position = body->GetPosition();
    JPH::Quat orientation = body->GetRotation();

    glm::vec3 pos(position.GetX(), position.GetY(), position.GetZ());
    glm::quat rot(orientation.GetW(), orientation.GetX(), orientation.GetY(), orientation.GetZ());

    Transform transform;
    transform.position = pos;
    transform.qrotation = rot;
    transform.CalcMatrix();
    return transform;
  }

  glm::vec3 JoltBody::GetPosition() const {
    JPH::RVec3 position = body->GetPosition();
    return glm::vec3(position.GetX(), position.GetY(), position.GetZ());
  }

  glm::quat JoltBody::GetOrientation() const {
    JPH::Quat orientation = body->GetRotation();
    return glm::quat(orientation.GetW(), orientation.GetX(), orientation.GetY(), orientation.GetZ());
  }

  Transform JoltBody::InterpolateTransform(float alpha) {
    JPH::RVec3 position = body->GetPosition();
    JPH::Quat orientation = body->GetRotation();

    glm::vec3 pos(position.GetX(), position.GetY(), position.GetZ());
    glm::quat rot(orientation.GetW(), orientation.GetX(), orientation.GetY(), orientation.GetZ());

    Transform transform;
    transform.position = pos;
    transform.qrotation = rot;
    transform.CalcMatrix();
    return transform;
  }

  void JoltBody::OnAddCollider(Ref<PhysicsShape> shape) {
  }

  void JoltBody::OnRemoveCollider(Ref<PhysicsShape> shape) {}

}  // namespace other