/**
 * \file physics/3D/react/react_body.cpp
 **/
#include "physics/3D/react/react_body.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "physics/3D/react/react_shape.hpp"

namespace other {

  ReactBody::ReactBody(rp3d::RigidBody* body)
      : body(body) {
    body->setType(rp3d::BodyType::STATIC);
    body->setIsActive(true);

    inter_transform = body->getTransform();
  }

  ReactBody::~ReactBody() {}

  glm::vec3 ReactBody::GetPosition() const {
    rp3d::Vector3 position = body->getTransform().getPosition();
    return glm::vec3(position.x, position.y, position.z);
  }

  glm::quat ReactBody::GetOrientation() const {
    rp3d::Quaternion orientation = body->getTransform().getOrientation();
    return glm::quat(orientation.w, orientation.x, orientation.y, orientation.z);
  }

  Transform ReactBody::InterpolateTransform(const Transform& target, float alpha) const {
    const rp3d::Transform& physics_transform = body->getTransform();
    const rp3d::Transform& i_transform = rp3d::Transform::interpolateTransforms(inter_transform, physics_transform, alpha);

    rp3d::decimal new_model_mat[16];
    i_transform.getOpenGLMatrix(new_model_mat);

    Transform new_transform = {};
    new_transform.model_transform = glm::make_mat4(new_model_mat);
    new_transform.model_transform = glm::make_mat4(new_model_mat);
    new_transform.model_transform = glm::scale(new_transform.model_transform, target.scale);

    new_transform.erotation.y = asin(-new_transform.model_transform[0][2]);
    if (cos(new_transform.erotation.y) != 0) {
      new_transform.erotation.x = atan2(new_transform.model_transform[1][2], new_transform.model_transform[2][2]);
      new_transform.erotation.z = atan2(new_transform.model_transform[0][1], new_transform.model_transform[0][0]);
    } else {
      new_transform.erotation.x = atan2(-new_transform.model_transform[2][0], new_transform.model_transform[1][1]);
      new_transform.erotation.z = 0;
    }

    new_transform.position = glm::vec3{
      physics_transform.getPosition().x,
      physics_transform.getPosition().y,
      physics_transform.getPosition().z
    };
    return new_transform;
  }

  void ReactBody::AddCollider(Ref<PhysicsShape> shape) {
    Ref<ReactShape> react_shape = Ref<PhysicsShape>::Cast<ReactShape>(shape);
    body->addCollider(react_shape->shape, body->getTransform());
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

}  // namespace other