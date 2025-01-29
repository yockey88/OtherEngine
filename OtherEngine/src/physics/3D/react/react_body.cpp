/**
 * \file physics/3D/react/react_body.cpp
 **/
#include "physics/3D/react/react_body.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <reactphysics3d/mathematics/Vector3.h>

#include "core/logger.hpp"
#include "math/matrix_math.hpp"

#include "ecs/components/transform.hpp"

#include "physics/3D/react/react_shape.hpp"

namespace other {

  ReactBody::ReactBody(rp3d::RigidBody* body, rp3d::PhysicsWorld* world)
      : PhysicsBody(), body(body), world(world) {
    OE_ASSERT(body != nullptr, "Physics body is null");
    OE_ASSERT(world != nullptr, "Physics world is null");
    inter_transform = body->getTransform();
    SetNativeBody(body);
  }

  ReactBody::~ReactBody() {
    if (body != nullptr) {
      world->destroyRigidBody(body);
    }
    body = nullptr;
  }

  void ReactBody::OnSetNativeBody(void* body) {
    if (body == nullptr) {
      body = nullptr;
    } else {
      this->body = static_cast<rp3d::RigidBody*>(body);
    }
  }

  void ReactBody::SetTransform(const Transform& transform) {
    rp3d::Vector3 pos(transform.position.x, transform.position.y, transform.position.z);
    rp3d::Quaternion orientation(transform.qrotation.x, transform.qrotation.y, transform.qrotation.z, transform.qrotation.w);
    rp3d::Transform new_transform(pos, orientation);

    body->setTransform(new_transform);
    inter_transform = new_transform;
    if (collider != nullptr) {
      rp3d::Transform local_transform(rp3d::Vector3(0.f, 0.f, 0.f), body->getTransform().getOrientation());
      collider->setLocalToBodyTransform(local_transform);
    }
  }

  Transform ReactBody::GetTransform() const {
    return ExtractTransform(body->getTransform());
  }

  glm::vec3 ReactBody::GetPosition() const {
    rp3d::Vector3 position = body->getTransform().getPosition();
    return glm::vec3(position.x, position.y, position.z);
  }

  glm::quat ReactBody::GetOrientation() const {
    rp3d::Quaternion orientation = body->getTransform().getOrientation();
    return glm::quat(orientation.w, orientation.x, orientation.y, orientation.z);
  }

  Transform ReactBody::InterpolateTransform(float alpha) {
    const rp3d::Transform& physics_transform = body->getTransform();
    const rp3d::Transform& i_transform = rp3d::Transform::interpolateTransforms(inter_transform, physics_transform, alpha);
    inter_transform = i_transform;
    return ExtractTransform(i_transform);
  }

  void ReactBody::SetVelocity(const glm::vec3& velocity) {
    rp3d::Vector3 vel = rp3d::Vector3{ velocity.x, velocity.y, velocity.z };
    body->setLinearVelocity(vel);
  }

  glm::vec3 ReactBody::GetVelocity() const {
    const rp3d::Vector3& velocity = body->getLinearVelocity();
    return glm::vec3{
      velocity.x,
      velocity.y,
      velocity.z,
    };
  }

  void ReactBody::SetAngularVelocity(const glm::vec3& velocity) {
    const rp3d::Vector3& vel = rp3d::Vector3{ velocity.x, velocity.y, velocity.z };
    body->setAngularVelocity(vel);
  }

  glm::vec3 ReactBody::GetAngularVelocity() const {
    const rp3d::Vector3& velocity = body->getAngularVelocity();
    return glm::vec3{
      velocity.x,
      velocity.y,
      velocity.z,
    };
  }

  void ReactBody::SetMass(float mass) {
    body->setMass(mass);
  }

  float ReactBody::GetMass() const {
    return body->getMass();
  }

  void ReactBody::SetLinearDrag(float drag) {
    body->setLinearDamping(drag);
  }

  float ReactBody::GetLinearDrag() const {
    return body->getLinearDamping();
  }

  void ReactBody::SetAngularDrag(float drag) {
    body->setAngularDamping(drag);
  }

  float ReactBody::GetAngularDrag() const {
    return body->getAngularDamping();
  }

  void ReactBody::SetGravityScale(float scale) {
    // body->setGravityScale(scale);
  }

  float ReactBody::GetGravityScale() const {
    // return body->getGravityScale();
    return 1.f;
  }

  void ReactBody::SetFixedRotation(bool fixed) {
    body->setIsAllowedToSleep(fixed);
  }

  bool ReactBody::GetFixedRotation() const {
    return body->isAllowedToSleep();
  }

  void ReactBody::SetBullet(bool bullet) {
    // body->setIsBullet(bullet);
  }

  bool ReactBody::GetBullet() const {
    // return body->isBullet();
    return false;
  }

  void ReactBody::SetTrigger(bool trigger) {
    // body->setTrigg
  }

  bool ReactBody::IsTrigger() const {
    // return body->isTrigger();
    return false;
  }

  void ReactBody::OnAddCollider(Ref<PhysicsShape> shape) {
    OE_ASSERT(body != nullptr, "Physics body is null");
    // OE_ASSERT(collider == nullptr, "Physics collider is not null (only one collider supported currently [React])");

    switch (shape->ShapeType()) {
      case PhysicsShape::Shape::BOX: {
        Ref<ReactBoxShape> box_shape = Ref<PhysicsShape>::Cast<ReactBoxShape>(shape);
        OE_ASSERT(box_shape != nullptr, "Box shape is null");
        OE_ASSERT(box_shape->shape != nullptr, "Box shape is null");

        rp3d::Transform local_transform(rp3d::Vector3(0.f, 0.f, 0.f), body->getTransform().getOrientation());
        collider = body->addCollider(box_shape->shape, local_transform);
      } break;

      case PhysicsShape::Shape::SPHERE: {
        Ref<ReactSphereShape> sphere_shape = Ref<PhysicsShape>::Cast<ReactSphereShape>(shape);
        OE_ASSERT(sphere_shape != nullptr, "Sphere shape is null");
        OE_ASSERT(sphere_shape->shape != nullptr, "Sphere shape is null");

        rp3d::Transform local_transform(rp3d::Vector3(0.f, 0.f, 0.f), body->getTransform().getOrientation());
        collider = body->addCollider(sphere_shape->shape, local_transform);
      } break;

      case PhysicsShape::Shape::CAPSULE: {
        Ref<ReactCapsuleShape> capsule_shape = Ref<PhysicsShape>::Cast<ReactCapsuleShape>(shape);
        OE_ASSERT(capsule_shape != nullptr, "Capsule shape is null");
        OE_ASSERT(capsule_shape->shape != nullptr, "Capsule shape is null");

        rp3d::Transform local_transform(rp3d::Vector3(0.f, 0.f, 0.f), body->getTransform().getOrientation());
        collider = body->addCollider(capsule_shape->shape, local_transform);
      } break;

      case PhysicsShape::Shape::CONVEX_MESH: {
        Ref<ReactConvexMeshShape> convex_mesh_shape = Ref<PhysicsShape>::Cast<ReactConvexMeshShape>(shape);
        OE_ASSERT(convex_mesh_shape != nullptr, "Convex mesh shape is null");
        OE_ASSERT(convex_mesh_shape->shape != nullptr, "Convex mesh shape is null");

        rp3d::Quaternion orientation = body->getTransform().getOrientation();
        rp3d::Quaternion inv_orientation = orientation.getInverse();

        rp3d::Quaternion actual_orientation = inv_orientation * orientation;

        rp3d::Transform local_transform(rp3d::Vector3(0.f, 0.f, 0.f), actual_orientation);
        body->addCollider(convex_mesh_shape->shape, local_transform);
      } break;

        // case PhysicsShape::Shape::CONCAVE_MESH: {
        //   Ref<ReactConcaveMeshShape> concave_mesh_shape = Ref<PhysicsShape>::Cast<ReactConcaveMeshShape>(shape);
        //   body->addCollider(concave_mesh_shape->shape, body->getTransform());
        // } break;

        // case PhysicsShape::Shape::COMPOUND_SHAPE: {
        //   Ref<CompoundShape> shape = Ref<PhysicsShape>::Cast<CompoundShape>(shape);
        //   OE_ASSERT(shape != nullptr, "Compound shape is null");

        //   for (auto& sub_shape : shape->shapes) {
        //     OnAddCollider(sub_shape);
        //   }
        // } break;

      default:
        OE_WARN("Physics Shape unimplemented!");
        break;
    }
  }

  void ReactBody::OnRemoveCollider(Ref<PhysicsShape> shape) {
    OE_ASSERT(shape != nullptr, "Physics shape is null");
    OE_ASSERT(body != nullptr, "Physics body is null");

    body->removeCollider(collider);
    collider = nullptr;
  }

  void ReactBody::OnBodyTypeChange(PhysicsBodyType type) {
    switch (type) {
      case STATIC:
        body->setType(rp3d::BodyType::STATIC);
        break;
      case DYNAMIC:
        body->setType(rp3d::BodyType::DYNAMIC);
        break;
      case KINEMATIC:
        body->setType(rp3d::BodyType::KINEMATIC);
        break;
      default:
        break;
    }
  }

  void ReactBody::OnSetEntity() {
    OE_ASSERT(body != nullptr, "Physics body is null");
    OE_ASSERT(entity_id.Get() != 0, "Entity ID is null");
    body->setUserData(&entity_id);
  }

  Transform ReactBody::ExtractTransform(const rp3d::Transform& physics_transform) const {
    float fmatrix[16];
    physics_transform.getOpenGLMatrix(fmatrix);

    glm::mat4 matrix = glm::make_mat4(fmatrix);

    glm::vec3 position;
    glm::quat rotation;
    [[maybe_unused]] glm::vec3 scale;

    DecomposeTransformMatrix(matrix, position, rotation, scale);

    Transform transform;
    transform.position = position;
    transform.qrotation = rotation;
    transform.CalcMatrix();
    return transform;
  }

}  // namespace other