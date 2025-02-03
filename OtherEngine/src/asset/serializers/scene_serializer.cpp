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
#include "core/view.hpp"

#include "application/app_state.hpp"
#include "parsing/ini_parser.hpp"

#include "ecs/component.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/collider_2d.hpp"
#include "ecs/components/light_source.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/physics_component.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/serialization_data.hpp"
#include "ecs/components/terrain.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/entity_serializer.hpp"
#include "ecs/systems/component_database.hpp"
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

    Ref<Scene> sref = Ref<Asset>::Cast<Scene>(metadata.asset);
    View<Scene> scene = View<Scene>{ sref };
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

    enum FileFormat {
      TEXT,
      BINARY,
    };

    FileFormat format = FileFormat::TEXT;
    if (scene_path.extension() == ".oscn") {
      format = FileFormat::BINARY;
    } else {
      OE_ASSERT(scene_path.extension() == ".yscn", "Scene file is not a scene file : {}", scene_path);
    }

    DeserializedScene scene_metadata;

    std::string name = "";
    UUID handle = 0;

    switch (format) {
      case FileFormat::TEXT: {
        try {
          IniFileParser parser{ scene_path.string() };
          scene_metadata.scene_table = parser.Parse();
        } catch (IniException& err) {
          OE_WARN("Failed to parse scene file - {} : {}", scene_path, err.what());
          return false;
        }
        OE_TRACE("Loading Scene :\n{}", scene_metadata.scene_table.TableString());

        scene_metadata.scene = NewRef<Scene>();
        scene_metadata.scene->scene_name = scene_metadata.scene_table.GetVal<std::string>(kMetadataSection, kNameValue, false).value_or(scene_path.stem().string());
        OE_ASSERT(!scene_metadata.scene->scene_name.empty(), "Scene name is empty : {}", scene_path);
        scene_metadata.scene->scene_handle = FNV(scene_metadata.name);
        scene_metadata.path = scene_path;

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
        metadata.asset = Ref<Asset>{ scene_metadata.scene };
      } break;

      /// FIXME: this needs to create a scene table from the binary data, it's not enough to just build the scene
      ///         maybe this could be done lazily?
      case FileFormat::BINARY: {
        ByteBuffer scene_buffer;
        {
          std::ifstream file(scene_path, std::ios::binary);
          if (!file.is_open()) {
            OE_ERROR("Failed to open scene file : {}", scene_path);
            return false;
          }
          {
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<uint8_t> buffer;
            buffer.resize(size);
            file.read((char*)buffer.data(), size);

            scene_buffer.Write(buffer.data(), buffer.size());
            file.close();
          }
          if (scene_buffer.Empty()) {
            return false;
          }
          OE_TRACE("Scene Buffer : {}", scene_buffer.DumpBuffer());
        }

        scene_metadata.scene = NewRef<Scene>();
        scene_metadata.scene->physics_world_2d = PhysicsWorld2D::Create(glm::vec2(0.f, -9.8f));
        scene_metadata.scene->physics_world = PhysicsWorld::Create(scene_metadata.scene.Raw());
        Read(scene_buffer, scene_metadata.scene);

        scene_metadata.path = scene_path;
        metadata.asset = Ref<Asset>{ scene_metadata.scene };
      } break;

      default:
        OE_ERROR("Unknown scene file format : {}", scene_path);
        return false;
    }

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

  void SceneSerializer::Write(ByteBuffer& buffer, const Ref<Scene>& scene) {
    OE_ASSERT(scene != nullptr, "Attempting to write a null scene");
    OE_DEBUG("Compiling scene : {}", scene->Name());

    if (!buffer.Empty()) {
      buffer.Release();
    }

    std::string name = scene->Name();
    OE_ASSERT(name.size() < kMaxNameLength, "Scene name is too long : {}", name.size());
    const auto& entities = scene->SceneEntities();

    const size_t scene_header_offset = 0;
    const size_t layout_table_offset = sizeof(SceneMeta);

    /// TODO: calculate group offsets, ent group might not be first page
    const size_t ent_group_offset = sizeof(SceneHeader);
    const size_t begin_of_entity_list = ent_group_offset + sizeof(uint64_t);
    const size_t ent_list_len = entities.size() * sizeof(EntityDescriptor);
    const size_t comp_tables_start_offset = begin_of_entity_list + ent_list_len;
    const size_t component_lists_start_offset = comp_tables_start_offset + entities.size() * sizeof(ComponentTable);

    OE_TRACE(" > Layout table offset : {}", layout_table_offset);
    OE_TRACE(" > Entity group offset : {}", ent_group_offset);
    OE_TRACE(" > Begin of entity list : {}", begin_of_entity_list);
    OE_TRACE(" > Entity list length : {}", ent_list_len);
    OE_TRACE(" > Component tables start offset : {}", comp_tables_start_offset);
    OE_TRACE(" > Component lists start offset : {}", component_lists_start_offset);

    buffer.BufferData(SceneHeader{});

    /// TODO: refactor out page-specific writing
    /// Beginning of physics page
    // buffer.BufferData(PhysicsHeader{}); // 2D and 3D

    /// Beginning of Entity page
    buffer.BufferData(EntityGroupHeader{});
    for (auto& [id, entity] : entities) {
      buffer.BufferData(EntityDescriptor{});
    }
    for (auto& [id, entity] : entities) {
      buffer.BufferData(ComponentTable{});
    }

    /// write header
    {
      SceneMeta& header = buffer.Read<SceneMeta>(scene_header_offset);
      header.name_len = static_cast<uint16_t>(name.size()),

      std::memset(header.name, 0, kMaxNameLength);
      std::memcpy(header.name, name.c_str(), name.size());
    }

    // write layout table
    {
      LayoutTable& layout = buffer.Read<LayoutTable>(layout_table_offset);
      layout.num_groups = 1;
      layout.groups[0] = /* entity group */ {
        .group_id = ENTITY_GROUP,
        .offset = ent_group_offset,
      };
    }

    /// Entity list
    {
      EntityGroupHeader& entity_data = buffer.Read<EntityGroupHeader>(ent_group_offset);
      entity_data.num_entities = entities.size();

      size_t ent_desc_cursor = begin_of_entity_list;
      size_t comp_table_cursor = comp_tables_start_offset;
      size_t comp_list_cursor = component_lists_start_offset;

      /// write entity descriptors
      for (auto& [id, entity] : entities) {
        OE_ASSERT(entity->Name().size() < kMaxNameLength, "Entity name is too long : {}", entity->Name().size());
        SerializationData& data = entity->GetComponent<SerializationData>();
        EntityDescriptor& descriptor = buffer.Read<EntityDescriptor>(ent_desc_cursor);
        {
          ComponentTable& comp_table = buffer.Read<ComponentTable>(comp_table_cursor);

          descriptor.entity_id = id.Get();
          descriptor.component_table_offset = comp_table_cursor;
          descriptor.component_list_offset = comp_list_cursor;
          descriptor.name_len = static_cast<uint16_t>(entity->Name().size());

          std::memset(descriptor.name, 0, kMaxNameLength);
          std::memcpy(descriptor.name, entity->Name().c_str(), descriptor.name_len);

          /// +3 to account for implicit components
          comp_table.num_components = data.entity_components.size() + 3u;
          for (int32_t j = 0; j < NUM_COMPONENTS; ++j) {
            comp_table.components[j].flag = 0;
          }
          /// these are always one for every entity
          comp_table.components[TAG_COMPONENT_INDEX].flag = 1;
          comp_table.components[TRANSFORM_COMPONENT_INDEX].flag = 1;
          comp_table.components[RELATIONSHIP_COMPONENT_INDEX].flag = 1;

          descriptor.component_list_len = 0;
          descriptor.component_list_len += TransformSnapshotter::Stride();
          descriptor.component_list_len += RelationshipSnapshotter::Stride();

          /// write rest of components here
          for (int32_t idx : data.entity_components) {
            OE_ASSERT(idx != 0, "Serialization should not save implicit components : {}", ComponentIndex{ idx });
            OE_ASSERT(idx != 1, "Serialization should not save implicit components : {}", ComponentIndex{ idx });
            OE_ASSERT(idx != 2, "Serialization should not save implicit components : {}", ComponentIndex{ idx });
            OE_ASSERT(idx < NUM_COMPONENTS, "Component index out of bounds : {}", idx);
            comp_table.components[idx].flag = 1;

            descriptor.component_list_len += ComponentDataBase::GetComponentStride(idx);
          }
        }

        OE_TRACE("  > Writing entity : {}", entity->Name());
        OE_TRACE("    > name len : {}", descriptor.name_len);
        OE_TRACE("    > id : {}", descriptor.entity_id);
        OE_TRACE("    > component table offset : {}", descriptor.component_table_offset);
        OE_TRACE("    > component list offset : {}", descriptor.component_list_offset);
        OE_TRACE("    > component list len : {}", descriptor.component_list_len);

        ent_desc_cursor += sizeof(EntityDescriptor);
        comp_table_cursor += sizeof(ComponentTable);

        OE_TRACE("COMPONENT CURSOR START = {} \\ end of components = {} \\ buffer size = {}", comp_list_cursor, comp_list_cursor + descriptor.component_list_len, buffer.Size());

        /// write components
        /// no tag because included in descriptor, field and relationship are implicit, hardcode their serialization
        {
          TransformSnapshotter transform_writer{};
          transform_writer.Write(buffer, entity->GetComponent<Transform>());
          comp_list_cursor += transform_writer.Stride();
        }
        OE_TRACE("  > Transform COMPONENT CURSOR = {} \\ end of components = {} \\ buffer size = {}", comp_list_cursor, comp_list_cursor + descriptor.component_list_len, buffer.Size());

        {
          RelationshipSnapshotter relationship_writer{};
          relationship_writer.Write(buffer, entity->GetComponent<Relationship>());
          comp_list_cursor += relationship_writer.Stride();
        }
        OE_TRACE("  > Relationship COMPONENT CURSOR = {} \\ end of components = {} \\ buffer size = {}", comp_list_cursor, comp_list_cursor + descriptor.component_list_len, buffer.Size());

        for (int32_t idx : entity->GetComponent<SerializationData>().entity_components) {
          if (idx == TAG_COMPONENT_INDEX || idx == TRANSFORM_COMPONENT_INDEX || idx == RELATIONSHIP_COMPONENT_INDEX) {
            continue;
          }

          Component* comp = entity->GetComponentByIndex(idx);
          if (comp == nullptr) {
            OE_ERROR("Component is null for entity : {}", entity->Name());
            continue;
          }

          comp->Serialize(buffer);
          comp_list_cursor += ComponentDataBase::GetComponentStride(idx);
          OE_TRACE("  > {} COMPONENT CURSOR = {} \\ end of components = {} \\ buffer size = {}", idx, comp_list_cursor, comp_list_cursor + descriptor.component_list_len, buffer.Size());
        }
      }
    }
    /// end of entity page
  }

  void SceneSerializer::Read(ByteBuffer& buffer, Ref<Scene>& scene) {
    OE_ASSERT(scene != nullptr, "Attempting to read into a null scene");

    if (buffer.Empty()) {
      OE_ERROR("Attempting to read from an empty buffer");
      return;
    }

    SceneHeader& header = buffer.Read<SceneHeader>(0);
    scene->scene_name = header.meta.name;
    scene->scene_handle = FNV(header.meta.name);

    OE_TRACE("Reading scene : {}", header.meta.name);
    if (header.layout.num_groups == 0) {
      OE_ERROR("No groups found in scene buffer");
      return;
    }
    OE_TRACE("  > {} groups", header.layout.num_groups);

    size_t cursor = header.layout.groups[0].offset;

    for (uint16_t i = 0; i < header.layout.num_groups; ++i) {
      GroupIndex& group = header.layout.groups[i];
      OE_TRACE("Reading group : {}", group.group_id);
      size_t group_cursor = group.offset;

      switch (group.group_id) {
        case ENTITY_GROUP: {
          EntityGroupHeader& entity_data = buffer.Read<EntityGroupHeader>(cursor);
          group_cursor = cursor + sizeof(uint64_t);  /// start of entities

          OE_TRACE("  > {} entities", entity_data.num_entities);
          for (uint64_t j = 0; j < entity_data.num_entities; ++j) {
            EntityDescriptor& descriptor = buffer.Read<EntityDescriptor>(group_cursor);
            ComponentTable& comp_table = buffer.Read<ComponentTable>(descriptor.component_table_offset);

            Entity* entity = scene->CreateEntity(descriptor.name);
            OE_ASSERT(entity != nullptr, "Failed to create entity : {}", descriptor.name);

            {
              Tag& tag = entity->GetComponent<Tag>();
              tag.id = descriptor.entity_id;
              tag.name = descriptor.name;
            }

            size_t beginning_of_components = descriptor.component_list_offset;
            size_t end_of_components = descriptor.component_list_offset + descriptor.component_list_len;
            size_t comp_cursor = beginning_of_components;

            OE_TRACE("  > Reading entity : {}", descriptor.name);
            OE_TRACE("    > name len : {}", descriptor.name_len);
            OE_TRACE("    > id : {}", descriptor.entity_id);
            OE_TRACE("    > component table offset : {}", descriptor.component_table_offset);
            OE_TRACE("    > component list offset : {}", descriptor.component_list_offset);
            OE_TRACE("    > component list len : {}", descriptor.component_list_len);
            OE_TRACE("    > end of components : {}", descriptor.component_list_offset + descriptor.component_list_len);

            OE_TRACE("    > Component table ({} components) :", comp_table.num_components);
            for (int32_t k = 0; k < NUM_COMPONENTS; ++k) {
              if (comp_table.components[k].flag == 1) {
                OE_TRACE("      > Component : {}", k);
              }
            }

            OE_TRACE("COMPONENT CURSOR START = {} \\ end of components = {}", comp_cursor, end_of_components);

            // have to have transform and relationship
            {
              TransformSnapshotter transform_reader{};
              Transform& transform = entity->GetComponent<Transform>();
              transform = transform_reader.Read(buffer, comp_cursor);
              comp_cursor += transform_reader.Stride();
            }
            OE_ASSERT(comp_cursor == beginning_of_components + TransformSnapshotter::Stride(), "Component cursor out of bounds");
            OE_TRACE("  > COMPONENT CURSOR = {} \\ end of components = {}", comp_cursor, end_of_components);

            {
              RelationshipSnapshotter relationship_reader{};
              Relationship& relationship = entity->GetComponent<Relationship>();
              relationship = relationship_reader.Read(buffer, comp_cursor);
              comp_cursor += relationship_reader.Stride();
            }
            OE_ASSERT(comp_cursor == beginning_of_components + TransformSnapshotter::Stride() + RelationshipSnapshotter::Stride(), "Component cursor out of bounds");
            OE_TRACE("  > COMPONENT CURSOR = {} \\ end of components = {}", comp_cursor, end_of_components);

            // read rest of components
            for (uint32_t c = MESH_COMPONENT_INDEX; c < NUM_COMPONENTS; ++c) {
              if (comp_cursor >= buffer.Size()) {
                OE_ASSERT(comp_cursor == end_of_components, "Component data mismatch for entity: {}", descriptor.name);
                break;
              }
              OE_TRACE("  > COMPONENT CURSOR = {} \\ end of components = {}", comp_cursor, end_of_components);

              if (comp_table.components[c].flag == 1) {
                OE_TRACE("    > Reading component : {}", c);
                switch (c) {
                  case MESH_COMPONENT_INDEX: {
                    MeshSnapshotter mesh_reader{};
                    entity->AddComponent<Mesh>(mesh_reader.Read(buffer, comp_cursor));
                    comp_cursor += mesh_reader.Stride();
                  } break;

                  case STATICMESH_COMPONENT_INDEX: {
                    StaticMeshSnapshotter static_mesh_reader{};
                    entity->AddComponent<StaticMesh>(static_mesh_reader.Read(buffer, comp_cursor));
                    comp_cursor += static_mesh_reader.Stride();
                  } break;

                  case SCRIPT_COMPONENT_INDEX: {
                    ScriptSnapshotter script_reader{};
                    entity->AddComponent<Script>(script_reader.Read(buffer, comp_cursor));
                    comp_cursor += script_reader.Stride();
                  } break;

                  case CAMERA_COMPONENT_INDEX: {
                    CameraSnapshotter camera_reader{};
                    entity->AddComponent<Camera>(camera_reader.Read(buffer, comp_cursor));
                    comp_cursor += camera_reader.Stride();
                  } break;

                  case RIGIDBODY2D_COMPONENT_INDEX: {
                    RigidBody2DSnapshotter rigidbody2d_reader{};
                    entity->AddComponent<RigidBody2D>(rigidbody2d_reader.Read(buffer, comp_cursor));
                    comp_cursor += rigidbody2d_reader.Stride();
                  } break;

                  case COLLIDER2D_COMPONENT_INDEX: {
                    Collider2DSnapshotter collider2d_reader{};
                    entity->AddComponent<Collider2D>(collider2d_reader.Read(buffer, comp_cursor));
                    comp_cursor += collider2d_reader.Stride();
                  } break;

                  case RIGIDBODY_COMPONENT_INDEX: {
                    RigidBodySnapshotter rigidbody_reader{};
                    entity->AddComponent<RigidBody>(rigidbody_reader.Read(buffer, comp_cursor));
                    comp_cursor += rigidbody_reader.Stride();
                  } break;

                  case COLLIDER_COMPONENT_INDEX: {
                    ColliderSnapshotter collider_reader{};
                    entity->AddComponent<Collider>(collider_reader.Read(buffer, comp_cursor));
                    comp_cursor += collider_reader.Stride();
                  } break;

                  case PHYSICS_OBJECT_COMPONENT_INDEX: {
                    [[maybe_unused]] PhysicsObject& physics_object = entity->AddComponent<PhysicsObject>();
                    RigidBody& body = entity->GetComponent<RigidBody>();
                    Collider& collider = entity->GetComponent<Collider>();

                    OE_ASSERT(body.physics_body != nullptr, "Physics body is null");
                    OE_ASSERT(collider.shape != nullptr, "Collider shape is null");

                    // RigidBodySnapshotter rigidbody_reader{};
                    // RigidBody rigidbody = rigidbody_reader.Read(buffer, comp_cursor);
                    // comp_cursor += rigidbody_reader.Stride();

                    // ColliderSnapshotter collider_reader{};
                    // Collider collider = collider_reader.Read(buffer, comp_cursor);
                    // comp_cursor += collider_reader.Stride();
                  } break;

                  case LIGHTSOURCE_COMPONENT_INDEX: {
                    LightSourceSnapshotter light_source_reader{};
                    entity->AddComponent<LightSource>(light_source_reader.Read(buffer, comp_cursor));
                    comp_cursor += light_source_reader.Stride();
                  } break;

                  case TERRAIN_COMPONENT_INDEX: {
                    TerrainSnapshotter terrain_reader{};
                    entity->AddComponent<Terrain>(terrain_reader.Read(buffer, comp_cursor));
                    comp_cursor += terrain_reader.Stride();
                  } break;

                  default:
                    OE_ERROR("Unknown component index : {}", c);
                    break;
                }
              }
            }
            OE_ASSERT(comp_cursor == end_of_components, "Component data mismatch for entity: {}", descriptor.name);
            OE_TRACE("  > COMPONENT CURSOR = {} \\ end of components = {}", comp_cursor, end_of_components);

            group_cursor += sizeof(EntityDescriptor);
          }
        } break;
        default:
          OE_ERROR("Unknown group id : {}", group.group_id);
          break;
      }
    }
  }

}  // namespace other
