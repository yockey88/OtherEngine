
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
    OE_ASSERT(entity->HasComponent<LightSource>() , "ENTITY WITHOUT TRANSFORM BEING DESERIALIZED");
    std::string comp_key = GetComponentSectionKey(entity->Name() , std::string{ kLightSourceValue });
  }

} // namespace other
