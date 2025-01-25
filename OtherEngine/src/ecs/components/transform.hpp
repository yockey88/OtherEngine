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
    glm::quat qrotation = glm::identity<glm::quat>();
    glm::mat4 model_transform = glm::identity<glm::mat4>();
    BBox bbox = BBox::empty;

    Transform(const glm::vec3& position);
    Transform(float p);
    Transform(float x, float y, float z);
    [[maybe_unused]] const glm::mat4& CalcMatrix();
    void Rotate(float angle, const glm::vec3& axis);

    ECS_COMPONENT(Transform, TRANSFORM_COMPONENT_INDEX);
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
