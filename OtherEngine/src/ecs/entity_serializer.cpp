/**
 * \file ecs/entity_serializer.cpp
 **/
#include "ecs/entity_serializer.hpp"
#include "core/config_keys.hpp"

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"
#include "ecs/components/serialization_data.hpp"
#include "ecs/systems/entity_serialization.hpp"

namespace other {

  void EntitySerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& ctx) const {
    if (entity == nullptr) {
      OE_ERROR("Attempting to serialize a null entity");
      return;
    }

    auto& tag = entity->GetComponent<Tag>();
    stream << "[" << tag.name << "]\n";
    stream << "UUID = " << FNV(tag.name) << "\n"; 
    stream << "components = {";

    const auto& serialization_data = entity->GetComponent<SerializationData>();
    const auto& comp_idxs = serialization_data.entity_components;
    
    if (comp_idxs.empty()) {
      stream << "}\n\n";
    } else {
      stream << "\n";
      for (auto itr = comp_idxs.begin(); itr != comp_idxs.end();) {
        stream << "  " << ComponentDataBase::GetComponentTagLc(*itr);

        ++itr;
        if (itr != comp_idxs.end()) {
          stream << " , ";
        }
        stream << "\n";
      }
      stream << "}\n\n";
    }

    auto transform_serializer = EntitySerialization::GetComponentSerializer(kTransformIndex);
    OE_ASSERT(transform_serializer != nullptr , "Failed to retrive transffomr serializer!");
    transform_serializer->Serialize(stream , entity , ctx);
    
    auto relationship_serializer = EntitySerialization::GetComponentSerializer(kRelationshipIndex);
    OE_ASSERT(relationship_serializer != nullptr , "Failed to retrive relationship serializer!");
    relationship_serializer->Serialize(stream , entity , ctx);

    for (const auto& i : comp_idxs) {
      auto serializer = EntitySerialization::GetComponentSerializer(i);
      OE_ASSERT(serializer != nullptr , "Component with index {} returned a null serializer");
      serializer->Serialize(stream , entity , ctx);
      stream << "\n";
    }
  }
  
  UUID EntitySerializer::Deserialize(Ref<Scene>& ctx , const std::string& name , const ConfigTable& scene_table) const {
    auto entity_data = scene_table.Get(name);

    Entity* entity = ctx->CreateEntity(name);
    UUID id = entity->GetComponent<Tag>().id;

    OE_ASSERT(entity != nullptr , "Failed to deserialize and create entity : [{} | {}]" , id.Get() , name);
    
    if (entity_data.empty()) {
      return id;
    }

    { /// transform
      Scope<ComponentSerializer> transform_serializer = EntitySerialization::GetComponentSerializer(kTransformIndex);
      OE_ASSERT(transform_serializer != nullptr , "Failed to retrieve transform serializer for deserialization");
      transform_serializer->Deserialize(entity , scene_table , ctx);
    }

    { /// relationship
      Scope<ComponentSerializer> relationship_serializer = EntitySerialization::GetComponentSerializer(kRelationshipIndex);
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
      OE_ASSERT(comp_serializer != nullptr , "Failed to retrieve serializer for component : [{}.{}]" , entity->Name() , comp);

      comp_serializer->Deserialize(entity , scene_table , ctx);
    }

    return id;
  }

} // namespace other
