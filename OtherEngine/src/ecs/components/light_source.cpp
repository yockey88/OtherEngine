
/**
 * \file ecs/components/light_source.cpp
 **/
#include "ecs/components/light_source.hpp"

#include "core/config_keys.hpp"
#include "ecs/entity.hpp"

namespace other {

  void LightSourceSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    auto& light_source = entity->GetComponent<LightSource>();

    SerializeComponentSection(stream , entity , "light-source");
    stream << "\n";
  }
      
  void LightSourceSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize a light-source into null entity or scene!");
    std::string key_value = GetComponentSectionKey(entity->Name() , std::string{ kLightSourceValue });

    auto& light = entity->AddComponent<LightSource>();
    light.type = (LightSourceType)scene_table.GetVal<uint32_t>(key_value , kTypeValue).value_or(POINT_LIGHT_SRC);

    switch (light.type) {
      case DIRECTION_LIGHT_SRC: {
        light.direction_light = {};
      } break;
      case POINT_LIGHT_SRC: {
        light.pointlight = {};
        light.pointlight.position = glm::vec4(entity->ReadComponent<Transform>().position , 1.f);
      } break;
      default:
        return;
    }

    entity->UpdateComponent(light);
  }

} // namespace other
