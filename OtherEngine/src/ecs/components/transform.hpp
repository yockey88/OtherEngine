/**
 * \file ecs/components/transform.hpp
 **/
#ifndef OTHER_ENGINE_TRANSFORM_HPP
#define OTHER_ENGINE_TRANSFORM_HPP

#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Transform : public Component {
    glm::vec3 scale = glm::vec3(1.f);
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 erotation = glm::vec3(0.f);
    glm::quat qrotation = glm::quat(0.f , 0.f , 0.f , 0.f);
    glm::mat4 model_transform = glm::identity<glm::mat4>();

    Transform(const glm::vec3& position) 
      : Component(kTransformIndex) , position(position) {}
    Transform(float p) 
      : Component(kTransformIndex) , position(glm::vec3(p)) {}
    Transform(float x, float y, float z) 
      : Component(kTransformIndex) , position(glm::vec3(x, y, z)) {}

    [[ maybe_unused ]] const glm::mat4& CalcMatrix() {
      qrotation = glm::quat(erotation);
      model_transform = glm::translate(glm::mat4(1.f) , position) *
                        glm::scale(glm::mat4(1.f) , scale) *
                        glm::toMat4(qrotation);
      return model_transform;
    }

    void Rotate(float angle , const glm::vec3& axis) {
      glm::quat q = glm::angleAxis(angle , axis);
      qrotation = q * qrotation;
      erotation = glm::eulerAngles(qrotation);
    }

    ECS_COMPONENT(Transform , kTransformIndex);
  };

  class TransformSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(Transform);
  };

} // namespace other

#endif // !OTHER_ENGINE_TRANSFORM_HPP
