/**
 * \file scene/scene_serializer.cpp
 **/
#include "scene/scene_serializer.hpp"

#include "core/errors.hpp"
#include "core/config_keys.hpp"
#include "core/logger.hpp"

#include "ecs/entity_serializer.hpp"

#include "parsing/ini_parser.hpp"
#include "physics/phyics_engine.hpp"

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
    
    stream << "}\n";

    std::string physics_str;

    stream << "physics = ";
    switch (scene->physics_type) {
      case PhysicsType::PHYSICS_2D:
        physics_str = "2D";
      break;
      case PhysicsType::PHYSICS_3D:
        physics_str = "3D";
      break;
      default:
      break;
    }
    stream << physics_str << "\n";

    /// final metadata newline 
    stream << "\n";

    EntitySerializer serializer;
    for (const auto& [id , e] : entities) {
      serializer.Serialize(stream , e , scene);
    }

    stream << "[physics." << physics_str << "]\n"; 
    switch (scene->physics_type) {
      case PhysicsType::PHYSICS_2D:
        SerializeVec2(stream , "gravity" , scene->physics_world_2d->GetGravity());
      break;
      case PhysicsType::PHYSICS_3D:
      break;
      default:
        break;
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

    std::string physics = scene_metadata.scene_table.GetVal<std::string>(kMetadataSection , kPhysicsValue).value_or("3D");
    uint64_t physics_hash = FNV(physics);
    std::string physics_section = std::string{ kPhysicsValue } + ".";
    switch (physics_hash) {
      case FNV("2D"): {
        physics_section += "2D";
        scene_metadata.scene->physics_type = PHYSICS_2D;

        auto gravity = scene_metadata.scene_table.Get(physics_section , kGravityValue);
        if (gravity.size() != 2) {
          OE_ERROR("{} section has corrupt gravity value!" , physics_section);
        } else {
          glm::vec2 g;
          DeserializeVec2(gravity , g);
          scene_metadata.scene->physics_world_2d = PhysicsEngine::GetPhysicsWorld2D(g);
        }
      } break;
      case FNV("3D"):
        physics_section += "3D";
        scene_metadata.scene->physics_type = PHYSICS_3D;
        scene_metadata.scene->physics_world = PhysicsEngine::GetPhysicsWorld();
      break;
      default:
        break;
    }

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
