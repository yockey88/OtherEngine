/**
 * \file ecs/components/transform.cpp
 **/
#include "ecs/components/transform.hpp"

#include "core/config_keys.hpp"
#include "ecs/entity.hpp"

namespace other {

  void TransformSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    auto& transform = entity->GetComponent<Transform>();

    stream << "[" << entity->Name() << ".transform]\n";
    SerializeVec3(stream , "position" , transform.position);
    SerializeQuat(stream , "rotation" , transform.qrotation);
    SerializeVec3(stream , "scale" , transform.scale);
  }
      
  void TransformSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity->HasComponent<Transform>() , "ENTITY WITHOUT TRANSFORM BEING DESERIALIZED");
    std::string comp_key = entity->Name();
    std::transform(comp_key.begin() , comp_key.end() , comp_key.begin() , ::toupper);

    comp_key += "." + std::string{ kTransformSection };

    auto pos_value = scene_table.Get(comp_key , kPositionValue);
    auto rotation_value = scene_table.Get(comp_key , kRotationValue);
    auto scale_value = scene_table.Get(comp_key , kScaleValue);

    if (pos_value.size() == 0 && rotation_value.size() == 0 && scale_value.size() == 0) {
      return;
    }

    auto& transform = entity->GetComponent<Transform>();
    if (pos_value.size() > 0) {
      DeserializeVec3(pos_value , transform.position);
    }
    
    if (rotation_value.size() > 0) {
      OE_ASSERT(rotation_value.size() == 4 , "CORRUPT TRANSFORM ROTATION [{}]" , comp_key);
      DeserializeQuat(rotation_value , transform.qrotation);
    }
    
    if (scale_value.size() > 0) {
      OE_ASSERT(scale_value.size() == 3 , "CORRUPT TRANSFORM SCALE [{}]" , comp_key);
      DeserializeVec3(scale_value , transform.scale);
    }
  }

} // namespace other
