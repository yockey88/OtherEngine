/**
 * \file asset/serializers/scene_serializer.cpp
 **/
#include "asset/serializers/scene_serializer.hpp"

#include "core/config_keys.hpp"
#include "core/defines.hpp"
#include "core/errors.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "core/util.hpp"

#include "application/app_state.hpp"
#include "parsing/ini_parser.hpp"

#include "ecs/entity_serializer.hpp"
#include "scene/scene_manager.hpp"

#include "physics/physics_defines.hpp"

namespace other {

  void SceneSerializer::Serialize(const AssetMetadata& metadata) {
    OE_ASSERT(metadata.asset != nullptr, "Attempting to serialize a null asset");
    OE_ASSERT(metadata.type == AssetType::SCENE, "Attempting to serialize a non-scene asset as a scene");

    if (!metadata.loaded || metadata.memory_asset) {
      OE_ERROR("Attempting to serialize an unloaded or memory asset : {}", metadata.handle);
      return;
    }

    Ref<FileHandle> file = Filesystem::GetFile(metadata.file_handle);
    if (file == nullptr) {
      OE_ERROR("Failed to get file handle for asset : {}", metadata.file_handle);
      return;
    }

    file->Open(std::ios_base::out);
    if (!file->IsOpen()) {
      OE_ERROR("Failed to open file for writing : {}", file->AbsolutePath().string());
      return;
    }

    View<Scene> scene = View<Scene>{ metadata.asset };
    SceneMetadata* scene_metadata = AppState::Scenes()->GetSceneMetadata(scene->SceneHandle());
    /// can gaurantee this because asset was loaded
    OE_ASSERT(scene_metadata != nullptr, "Failed to get scene metadata for scene : {}", scene->SceneHandle());
    Serialize(scene_metadata->name, file->GetWriteStream(), Ref<Scene>{ scene });
  }

  bool SceneSerializer::Load(AssetMetadata& metadata) {
    OE_ASSERT(metadata.asset == nullptr, "Attempting to load a null asset");
    OE_ASSERT(metadata.type == AssetType::SCENE, "Attempting to load a non-scene asset as a scene");

    Path scene_path = metadata.path;
    if (!Filesystem::FileExists(scene_path)) {
      OE_ERROR("Scene file does not exist : {}", scene_path);
      return false;
    }

    DeserializedScene scene_metadata;
    try {
      IniFileParser parser{ scene_path.string() };
      scene_metadata.scene_table = parser.Parse();
    } catch (IniException& err) {
      OE_WARN("Failed to parse scene file - {} : {}", scene_path, err.what());
      return false;
    }
    OE_TRACE("Loading Scene :\n{}", scene_metadata.scene_table.TableString());

    scene_metadata.scene = NewRef<Scene>();

    scene_metadata.scene->scene_handle = FNV(scene_metadata.name);

    scene_metadata.path = scene_path;
    scene_metadata.name = scene_metadata.scene_table.GetVal<std::string>(kMetadataSection, kNameValue, false).value_or(scene_path.stem().string());
    OE_ASSERT(!scene_metadata.name.empty(), "Scene name is empty : {}", scene_path);

    scene_metadata.scene->scene_name = scene_metadata.name;

    std::string physics_section = std::string{ kPhysicsValue } + ".";

    std::string physics_2d_section = physics_section + "2D";
    std::string physics_3d_section = physics_section + "3D";

    auto gravity = scene_metadata.scene_table.Get(physics_2d_section, kGravityValue);
    if (gravity.size() != 2) {
      OE_ERROR("{} section has corrupt gravity value!", physics_2d_section);
    } else {
      glm::vec2 g;
      DeserializeVec2(gravity, g);
      scene_metadata.scene->physics_world_2d = PhysicsWorld2D::Create(g);
    }

    scene_metadata.scene->physics_world = PhysicsWorld::Create(scene_metadata.scene.Raw());

    EntitySerializer deserializer;
    auto entities = scene_metadata.scene_table.Get(kMetadataSection, kEntitiesValue);

    OE_INFO("Attempting to load {} entities", entities.size());
    for (auto& e : entities) {
      if (scene_metadata.scene->EntityExists(e)) {
        continue;
      }

      OE_DEBUG("Deserializing root entity : {}", e);
      deserializer.Deserialize(scene_metadata.scene, e, scene_metadata.scene_table);
    }

    OE_INFO("Scene loaded : {}", scene_path);
    metadata.asset = Ref<Asset>(scene_metadata.scene);
    AppState::Scenes()->AddScene(scene_metadata);
    return true;
  }

  void SceneSerializer::Serialize(const std::string_view scene_name, std::ostream& stream, const Ref<Scene>& scene) const {
    if (scene == nullptr) {
      OE_ERROR("Attempting to serialize scene {} with a null scene reference", scene_name);
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

    /// final metadata newline
    stream << "\n";

    EntitySerializer serializer;
    for (const auto& [id, e] : entities) {
      serializer.Serialize(stream, e, scene);
    }

    stream << "[physics.2D]\n";
    SerializeVec2(stream, "gravity", scene->physics_world_2d->GetGravity());
    stream << "\n";

    stream << "[physics.3D]\n";
    stream << "\n";
  }

}  // namespace other
