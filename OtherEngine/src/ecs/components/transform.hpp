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
      : position(position) {}
    Transform(float p) 
      : position(glm::vec3(p)) {}
    Transform(float x, float y, float z) 
      : position(glm::vec3(x, y, z)) {}

    ECS_COMPONENT(Transform);
  };

  class TransformSerializer : public ComponentSerializer {
    public:
      virtual ~TransformSerializer() override {}

      virtual void Serialize(std::ostream& stream , Entity* entity) const override;
      virtual void Deserialize(Entity* entity , const ConfigTable& scene_table) const override;
  };

} // namespace other

#endif // !OTHER_ENGINE_TRANSFORM_HPP
