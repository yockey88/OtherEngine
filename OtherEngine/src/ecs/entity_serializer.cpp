/**
 * \file ecs/entity_serializer.cpp
 **/
#include "ecs/entity_serializer.hpp"
#include "core/config_keys.hpp"
#include "ecs/component_serializer.hpp"
#include "ecs/systems/entity_serialization.hpp"

namespace other {

  void EntitySerializer::Serialize(std::ostream& stream , Entity* entity) const {

  }
  
  UUID EntitySerializer::Deserialize(Ref<Scene>& ctx , const std::string& name , const ConfigTable& scene_table) const {
    auto entity_data = scene_table.Get(name);

    UUID id = scene_table.GetVal<UUID>(name , kUuidValue).value_or(FNV(name));
    Entity* entity = ctx->CreateEntity(name , id);

    OE_ASSERT(entity != nullptr , "Failed to deserialize and create entity : [{} | {}]" , id.Get() , name);
    
    if (entity_data.empty()) {
      return id;
    }

    { /// transform
      Scope<ComponentSerializer> transform_serializer = EntitySerialization::GetComponentSerializer(kTransformSection);
      OE_ASSERT(transform_serializer != nullptr , "Failed to retrieve transform serializer for deserialization");
      transform_serializer->Deserialize(entity , scene_table , ctx);
    }

    { /// relationship
      Scope<ComponentSerializer> relationship_serializer = EntitySerialization::GetComponentSerializer(kRelationshipSection);
      OE_ASSERT(relationship_serializer != nullptr , "Failed to retrieve relationship serializer for deserialization");
      relationship_serializer->Deserialize(entity , scene_table , ctx);
    }

    auto components = scene_table.Get(name , kComponentsValue);
    if (components.empty()) {
      return id;
    }

    std::string key_name = name;
    std::transform(key_name.begin() , key_name.end() , key_name.begin() , ::toupper);
    for (auto& comp : components) {
      Scope<ComponentSerializer> comp_serializer = EntitySerialization::GetComponentSerializer(comp);
      if (comp_serializer == nullptr) {
        OE_ERROR("Failed to retrieve serializer for component : [{}.{}]" , entity->Name() , comp);
        continue;
      }

      comp_serializer->Deserialize(entity , scene_table , ctx);
    }

    return id;
  }

} // namespace other
