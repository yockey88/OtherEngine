
/**
 * \file ecs/components/point_light.cpp
 **/
#include "ecs/components/point_light.hpp"

#include "core/config_keys.hpp"
#include "ecs/entity.hpp"

namespace other {

  void PointLightSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    auto& transform = entity->GetComponent<PointLight>();

    SerializeComponentSection(stream , entity , "point-light");
    stream << "\n";
  }
      
  void PointLightSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity->HasComponent<PointLight>() , "ENTITY WITHOUT TRANSFORM BEING DESERIALIZED");
    std::string comp_key = GetComponentSectionKey(entity->Name() , std::string{ kPointLightValue });
  }

} // namespace other
