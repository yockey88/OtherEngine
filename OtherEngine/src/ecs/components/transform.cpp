/**
 * \file ecs/components/transform.cpp
 **/
#include "ecs/components/transform.hpp"

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/gtx/quaternion.hpp>

#include "core/config_keys.hpp"

#include "ecs/entity.hpp"
#include "scene/scene.hpp"

namespace other {

  Transform::Transform(const glm::vec3& scale, const glm::vec3& position, const glm::vec3& erotation)
      : Component(TRANSFORM_COMPONENT_INDEX), scale(scale), position(position), erotation(erotation) {
    bbox = BBox(position);
  }

  Transform::Transform(const glm::vec3& position)
      : Component(TRANSFORM_COMPONENT_INDEX), position(position) {
    bbox = BBox(position);
  }

  Transform::Transform(float p)
      : Component(TRANSFORM_COMPONENT_INDEX), position(glm::vec3(p)) {
    bbox = BBox(position);
  }

  Transform::Transform(float x, float y, float z)
      : Component(TRANSFORM_COMPONENT_INDEX), position(glm::vec3(x, y, z)) {
    bbox = BBox(position);
  }

  const glm::mat4& Transform::CalcMatrix() {
    glm::vec3 dim = scale * 0.5f;
    bbox = BBox(position - dim, position + dim);

    qrotation = glm::normalize(qrotation);
    erotation = glm::eulerAngles(qrotation);

    model_transform = glm::translate(glm::mat4(1.f), position) *
      glm::toMat4(qrotation) *
      glm::scale(glm::mat4(1.f), scale);
    /// normalize matrix to avoid scaling issues
    return model_transform;
  }

  void Transform::Rotate(float angle, const glm::vec3& axis) {
    glm::quat q = glm::angleAxis(angle, axis);
    qrotation = q * qrotation;
    erotation = glm::eulerAngles(qrotation);
  }

  TransformSnapshotter::TransformSnapshotter() {
    AddField<glm::vec3, 0>(&Transform::scale);
    AddField<glm::vec3, 1>(&Transform::position);
    AddField<glm::vec3, 2>(&Transform::erotation);
    AddField<glm::quat, 3>(&Transform::qrotation);
  }

  void TransformSerializer::Serialize(std::ostream& stream, Entity* entity, const Ref<Scene>& scene) const {
    auto& transform = entity->GetComponent<Transform>();

    SerializeComponentSection(stream, entity, "transform");
    SerializeVec3(stream, "position", transform.position);
    SerializeQuat(stream, "rotation", transform.qrotation);
    SerializeVec3(stream, "scale", transform.scale);
    stream << "\n";
  }

  void TransformSerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    OE_ASSERT(entity->HasComponent<Transform>(), "ENTITY WITHOUT TRANSFORM BEING DESERIALIZED");
    std::string comp_key = GetComponentSectionKey(entity->Name(), std::string{ kTransformSection });

    const auto& pos_value = scene_table.Get(comp_key, kPositionValue);
    const auto& rotation_value = scene_table.Get(comp_key, kRotationValue);
    const auto& scale_value = scene_table.Get(comp_key, kScaleValue);

    if (pos_value.size() == 0 && rotation_value.size() == 0 && scale_value.size() == 0) {
      return;
    }

    auto& transform = entity->GetComponent<Transform>();
    if (pos_value.size() > 0) {
      DeserializeVec3(pos_value, transform.position);
    }

    if (rotation_value.size() > 0) {
      OE_ASSERT(rotation_value.size() == 4, "CORRUPT TRANSFORM ROTATION [{}]", comp_key);
      DeserializeQuat(rotation_value, transform.qrotation);
    }

    if (scale_value.size() > 0) {
      OE_ASSERT(scale_value.size() == 3, "CORRUPT TRANSFORM SCALE [{}]", comp_key);
      DeserializeVec3(scale_value, transform.scale);

      if (transform.scale.x == 0) {
        transform.scale.x = 1;
      }

      if (transform.scale.y == 0) {
        transform.scale.y = 1;
      }

      if (transform.scale.z == 0) {
        transform.scale.z = 1;
      }
    }

    transform.CalcMatrix();
  }

}  // namespace other
