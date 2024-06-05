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

      comp_serializer->Deserialize(entity , scene_table);
    }

    return id;
  }

} // namespace other
