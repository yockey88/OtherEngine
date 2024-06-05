/**
 * \file ecs/components/transform.cpp
 **/
#include "ecs/components/transform.hpp"

#include "core/config_keys.hpp"
#include "ecs/entity.hpp"

namespace other {

  void TransformSerializer::Serialize(std::ostream& stream , Entity* entity) const {
    auto& transform = entity->GetComponent<Transform>();

    stream << "[" << entity->Name() << ".transform]\n";
    stream << "position = { " 
           << transform.position.x 
           << transform.position.y
           << transform.position.z
           << " }\n";
                            
    stream << "rotation = { " 
           << transform.qrotation.x 
           << transform.qrotation.y
           << transform.qrotation.z
           << transform.qrotation.w
           << " }\n";

    stream << "scale = { " 
           << transform.scale.x 
           << transform.scale.y
           << transform.scale.z
           << " }\n";
  }
      
  void TransformSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table) const {
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

    /// all entities already have a transform
    auto& transform = entity->GetComponent<Transform>();
    if (pos_value.size() > 0) {
      OE_ASSERT(pos_value.size() == 3 , "CORRUPT TRANSFORM POSITION [{}]" , comp_key);
      transform.position.x = std::stof(pos_value[0]);
      transform.position.y = std::stof(pos_value[1]);
      transform.position.z = std::stof(pos_value[2]);
    }
    
    if (rotation_value.size() > 0) {
      OE_ASSERT(rotation_value.size() == 4 , "CORRUPT TRANSFORM ROTATION [{}]" , comp_key);
      transform.qrotation.x = std::stof(rotation_value[0]);
      transform.qrotation.y = std::stof(rotation_value[1]);
      transform.qrotation.z = std::stof(rotation_value[2]);
      transform.qrotation.w = std::stof(rotation_value[3]);
    }
    
    if (scale_value.size() > 0) {
      OE_ASSERT(scale_value.size() == 3 , "CORRUPT TRANSFORM SCALE [{}]" , comp_key);
      transform.scale.x = std::stof(scale_value[0]);
      transform.scale.y = std::stof(scale_value[1]);
      transform.scale.z = std::stof(scale_value[2]);
    }
  }

} // namespace other
