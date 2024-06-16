/**
 * \file scene/scene_serializer.cpp
 **/
#include "scene/scene_serializer.hpp"

#include "core/errors.hpp"
#include "core/config_keys.hpp"
#include "ecs/entity_serializer.hpp"
#include "parsing/ini_parser.hpp"

namespace other {

  void SceneSerializer::Serialize(const std::string_view scene_name , std::ostream& stream , const Ref<Scene>& scene) const {
    if (scene == nullptr) {
      OE_ERROR("Attempting to serialize scene {} with a null scene reference" , scene_name);
      return;
    }
    
    const auto& entities = scene->SceneEntities();

    stream << "[metadata]\n";
    stream << "name = \"" << scene_name << "\"\n";
    stream << "entities = {\n";

    for (auto itr = entities.begin(); itr != entities.end();) {
      stream << "  \"" << (*itr).second->Name() << "\"";

      ++itr;

      if (itr != entities.end()) {
        stream << " , ";
      }
      stream << "\n";
    }
    
    stream << "}\n\n";

    EntitySerializer serializer;
    for (const auto& [id , e] : entities) {
      serializer.Serialize(stream , e , scene);
    }
  }

  DeserializedScene SceneSerializer::Deserialize(const std::string_view scn_path) const {
    DeserializedScene scene_metadata;
    try {
      IniFileParser parser{ scn_path.data() };
      scene_metadata.scene_table = parser.Parse();

    } catch (IniException& err) {
      OE_WARN("Failed to parse scene file - {} : {}" , scn_path , err.what()); 
      return {};
    }  

    scene_metadata.name = scene_metadata.scene_table.GetVal<std::string>(kMetadataSection , kNameValue).value_or(scn_path.data());
    scene_metadata.scene = NewRef<Scene>();

    EntitySerializer deserializer;

    auto entities = scene_metadata.scene_table.Get(kMetadataSection , kEntitiesValue);

    OE_INFO("Attempting to load {} entities" , entities.size());
    for (auto& e : entities) {
      if (scene_metadata.scene->EntityExists(e)) {
        continue;
      }

      OE_DEBUG("Deserializing root entity : {}" , e);

      /* UUID entity_id = */ deserializer.Deserialize(scene_metadata.scene , e , scene_metadata.scene_table);
    }

    OE_INFO("Scene loaded : {}" , scn_path);
    return scene_metadata;
  }

} // namespace other
