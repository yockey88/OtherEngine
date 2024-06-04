/**
 * \file ecs/entity_serializer.cpp
 **/
#include "ecs/entity_serializer.hpp"
#include "core/config_keys.hpp"

namespace other {

  void EntitySerializer::Serialize(std::ostream& stream , Entity* entity) const {

  }
  
  void EntitySerializer::Deserialize(Ref<Scene>& ctx , const std::string& name , const ConfigTable& scene_table) const {
    OE_DEBUG("Deserializing {}" , name);
    auto entity_data = scene_table.Get(name);

    UUID id = scene_table.GetVal<UUID>(name , kUuidValue).value_or(FNV(name));
    Entity* entity = ctx->CreateEntity(name , id);
    
    if (entity_data.empty()) {
      return;
    }

    auto components = scene_table.Get(name , kComponentsValue);
    if (components.empty()) {
      return;
    }

    std::string key_name = name;
    std::transform(key_name.begin() , key_name.end() , key_name.begin() , ::toupper);
    for (auto& comp : components) {
      std::string comp_key = fmt::format(fmt::runtime("{}.{}") , key_name , comp);
      auto comp_data = scene_table.Get(comp_key);
    }
  }

} // namespace other
