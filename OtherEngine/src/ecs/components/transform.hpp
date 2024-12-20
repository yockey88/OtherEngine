/**
 * \file ecs/components/transform.hpp
 **/
#ifndef OTHER_ENGINE_TRANSFORM_HPP
#define OTHER_ENGINE_TRANSFORM_HPP

#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <reflection/echo_defines.hpp>

#include "math/bounding_box.hpp"

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Transform : public Component {
    glm::vec3 scale = glm::vec3(1.f);
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 erotation = glm::vec3(0.f);
    glm::quat qrotation = glm::quat(0.f, 0.f, 0.f, 0.f);
    glm::mat4 model_transform = glm::identity<glm::mat4>();
    BBox bbox = BBox::empty;

    Transform(const glm::vec3& position)
        : Component(kTransformIndex), position(position) {
      bbox = BBox(position);
    }
    Transform(float p)
        : Component(kTransformIndex), position(glm::vec3(p)) {
      bbox = BBox(position);
    }
    Transform(float x, float y, float z)
        : Component(kTransformIndex), position(glm::vec3(x, y, z)) {
      bbox = BBox(position);
    }

    [[maybe_unused]] const glm::mat4& CalcMatrix() {
      glm::vec3 dim = scale * 0.5f;
      bbox = BBox(position - dim, position + dim);

      erotation = glm::eulerAngles(qrotation);
      model_transform = glm::translate(glm::mat4(1.f), position) * glm::mat4_cast(qrotation);
      model_transform = glm::scale(model_transform, scale);
      return model_transform;
    }

    void Rotate(float angle, const glm::vec3& axis) {
      glm::quat q = glm::angleAxis(angle, axis);
      qrotation = q * qrotation;
      erotation = glm::eulerAngles(qrotation);
    }

    ECS_COMPONENT(Transform, kTransformIndex);
  };

  class TransformSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(Transform);
  };

}  // namespace other

ECHO_TYPE(
  type(other::Transform),
  field(scale),
  field(position),
  field(erotation),
  field(qrotation),
  field(model_transform)
);

#endif  // !OTHER_ENGINE_TRANSFORM_HPP
