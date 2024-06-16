/**
 * \file ecs/components/relationship.cpp
 **/
#include "ecs/components/relationship.hpp"

#include "core/config_keys.hpp"
#include "ecs/entity.hpp"
#include "ecs/entity_serializer.hpp"
#include "ecs/components/tag.hpp"

namespace other {

  void RelationshipSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    OE_ASSERT(entity->HasComponent<Relationship>() , "Attempting to serialize an entity without a relationship");

    const auto& relationship = entity->ReadComponent<Relationship>();
    
    stream << "[" << entity->Name() << ".relationship]\n";
    stream << "children = {";
    if (relationship.children.empty()) {
      stream << "}\n\n";
      return;
    }
    stream << "\n";

    for (auto itr = relationship.children.begin(); itr != relationship.children.end();) {
      auto* child = scene->GetEntity(*itr);
      if (child == nullptr) {
        OE_ERROR("{} has invalid child with id : {}" , entity->Name() , itr->Get());
        continue;
      }

      stream << "  \"" << child->Name() << "\"";

      ++itr;

      if (itr != relationship.children.end()) {
        stream << " , ";
      }
      stream << "\n";
    }
    stream << "}\n\n";
  } 

  void RelationshipSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    std::string key_name = GetComponentSectionKey(entity->Name() , std::string{ kRelationshipSection });

    auto children = scene_table.Get(key_name , kChildrenValue);
    if (children.empty()) {
      return;
    }
    
    OE_DEBUG("  > Deserializing {} : {} children" , entity->Name() , children.size());

    EntitySerializer deserializer;

    for (auto& c : children) {
      const auto& loaded_ents = scene->SceneEntities();
      /// check if we loaded the entity as another entities child
      if (std::find_if(loaded_ents.begin() , loaded_ents.end() , [c](const auto& ent) -> bool {
        return c == ent.second->Name();
      }) != loaded_ents.end()) {
        continue;
      }
      OE_DEBUG("Deserializing entity child {}" , c);

      UUID child_id = deserializer.Deserialize(scene , c , scene_table);
      Entity* child = scene->GetEntity(child_id);

      child->GetComponent<Relationship>().parent = entity->GetComponent<Tag>().id;
      entity->GetComponent<Relationship>().children.push_back(child_id);

      OE_DEBUG("   > Parent = {} | Child = {}" , child->ReadComponent<Relationship>().parent.value().Get() , child_id.Get());
    }
  }

} // namespace other
