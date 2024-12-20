
/**
 * \file ecs/components/light_source.cpp
 **/
#include "ecs/components/light_source.hpp"

#include "core/config_keys.hpp"

#include "ecs/entity.hpp"

namespace other {

  // Material LightSource::debug_light_mat = Material({ 1.f, 1.f, 1.f, 1.f }, 32.f);

  void LightSourceSerializer::Serialize(std::ostream& stream, Entity* entity, const Ref<Scene>& scene) const {
    if (!entity->HasComponent<LightSource>()) {
      return;
    }

    auto& light_source = entity->GetComponent<LightSource>();
    SerializeComponentSection(stream, entity, "light-source");
    stream << "\n";
  }

  void LightSourceSerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr, "Attempting to deserialize a light-source into null entity or scene!");
    std::string key_value = GetComponentSectionKey(entity->Name(), std::string{ kLightSourceValue });

    auto& light = entity->AddComponent<LightSource>();
    light.type = (LightSourceType)scene_table.GetVal<uint32_t>(key_value, kTypeValue, false).value_or(POINT_LIGHT_SRC);

    auto color = scene_table.GetVal<glm::vec4>(key_value, kColorValue, false).value_or(glm::vec4{ 1.f, 1.f, 1.f, 1.f });

    switch (light.type) {
      case DIRECTION_LIGHT_SRC: {
        auto direction = scene_table.GetVal<glm::vec4>(key_value, kDirectionValue, false);
        light.direction_light = {
          .direction = direction.value_or(glm::vec4{ 0.f, -1.f, 0.f, 1.f }),
          .color = color,
        };

        auto& transform = entity->GetComponent<Transform>();
        transform.erotation = glm::vec3(light.direction_light.direction);
        transform.position = -glm::normalize(glm::vec3(light.direction_light.direction)) * 10.f;
        light.direction_light.position = glm::vec4(transform.position, 1.0);

        float near_plane = 0.1f, far_plane = 100.f;
        // glm::vec3 light_target = transform.position + glm::vec3(light.direction_light.direction);
        glm::mat4 light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 light_view = glm::lookAt(transform.position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
        light.direction_light.light_space_matrix = light_projection * light_view;
      } break;
      case POINT_LIGHT_SRC: {
        auto position = scene_table.GetVal<glm::vec4>(key_value, kPositionValue, false);
        auto radius = scene_table.GetVal<float>(key_value, kRadiusValue, false).value_or(100.f);
        auto constant = scene_table.GetVal<float>(key_value, kConstantValue, false).value_or(1.f);
        auto linear = scene_table.GetVal<float>(key_value, kLinearValue, false).value_or(0.09f);
        auto quadratic = scene_table.GetVal<float>(key_value, kQuadraticValue, false).value_or(0.032f);
        light.pointlight = {
          .position = position.value_or(glm::vec4{ 0.f, 0.f, 0.f, 1.f }),
          .color = color,
          .radius = radius,
          .constant = constant,
          .linear = linear,
          .quadratic = quadratic,
        };

        auto& transform = entity->GetComponent<Transform>();
        transform.position = glm::vec3(light.pointlight.position);
      } break;
      default:
        return;
    }

    entity->UpdateComponent(light);
  }  // namespace other

}  // namespace other
