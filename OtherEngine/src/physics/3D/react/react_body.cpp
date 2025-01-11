/**
 * \file physics/3D/react/react_body.cpp
 **/
#include "physics/3D/react/react_body.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "core/logger.hpp"
#include "math/matrix_math.hpp"

#include "ecs/components/transform.hpp"

#include "physics/3D/react/react_shape.hpp"

namespace other {

  ReactBody::ReactBody(rp3d::RigidBody* body)
      : body(body) {
    inter_transform = body->getTransform();
  }

  ReactBody::~ReactBody() {}

  void ReactBody::SetTransform(const Transform& transform) {
    rp3d::Vector3 pos(transform.position.x, transform.position.y, transform.position.z);
    rp3d::Quaternion orientation(transform.qrotation.x, transform.qrotation.y, transform.qrotation.z, transform.qrotation.w);
    rp3d::Transform new_transform(pos, orientation);

    body->setTransform(new_transform);
    inter_transform = new_transform;
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

  void ReactBody::AddCollider(Ref<PhysicsShape> shape) {
    switch (shape->ShapeType()) {
      case PhysicsShapeType::BOX: {
        Ref<ReactBoxShape> box_shape = Ref<PhysicsShape>::Cast<ReactBoxShape>(shape);
        rp3d::Transform local_transform(rp3d::Vector3(0.f, 0.f, 0.f), body->getTransform().getOrientation());
        body->addCollider(box_shape->shape, local_transform);
      } break;

      case PhysicsShapeType::SPHERE: {
        Ref<ReactSphereShape> sphere_shape = Ref<PhysicsShape>::Cast<ReactSphereShape>(shape);
        rp3d::Transform local_transform(rp3d::Vector3(0.f, 0.f, 0.f), body->getTransform().getOrientation());
        body->addCollider(sphere_shape->shape, local_transform);
      } break;

      case PhysicsShapeType::CAPSULE: {
        Ref<ReactCapsuleShape> capsule_shape = Ref<PhysicsShape>::Cast<ReactCapsuleShape>(shape);
        rp3d::Transform local_transform(rp3d::Vector3(0.f, 0.f, 0.f), body->getTransform().getOrientation());
        body->addCollider(capsule_shape->shape, local_transform);
      } break;

        // case PhysicsShapeType::CONVEX_MESH: {
        //   Ref<ReactConvexMeshShape> convex_mesh_shape = Ref<PhysicsShape>::Cast<ReactConvexMeshShape>(shape);
        //   body->addCollider(convex_mesh_shape->shape, body->getTransform());
        // } break;

        // case PhysicsShapeType::CONCAVE_MESH: {
        //   Ref<ReactConcaveMeshShape> concave_mesh_shape = Ref<PhysicsShape>::Cast<ReactConcaveMeshShape>(shape);
        //   body->addCollider(concave_mesh_shape->shape, body->getTransform());
        // } break;

      default:
        OE_WARN("Physics Shape unimplemented!");
        break;
    }
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