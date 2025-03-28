/**mesh.c
 * \file ecs/comopnents/mesh.cpp
 **/
#include "ecs/components/mesh.hpp"

#include <glm/ext/matrix_transform.hpp>

#include "core/config_keys.hpp"
#include "core/filesystem.hpp"

#include "asset/asset_manager.hpp"

#include "ecs/entity.hpp"
#include "scene/scene.hpp"

#include "rendering/model_factory.hpp"

#include "serialization/scene_file_format_defines.hpp"

namespace other {

  AssetHandle GetMeshHandle(const std::string& path) {
    Ref<Directory> assets = Filesystem::GetDirectory("assets");
    if (assets == nullptr) {
      OE_ERROR("Failed to retrieve assets directory");
      return 0;
    }

    Ref<FileHandle> file = assets->OpenFile(path);
    if (file == nullptr) {
      OE_ERROR("Failed to open mesh file {}", path);
      return 0;
    }

    Ref<ModelSource> asset = AssetManager::GetAsset<ModelSource>(file->handle, AssetType::MODEL_SOURCE);
    if (asset == nullptr) {
      OE_ERROR("Failed to retrieve asset handle for mesh {}", path);
      return 0;
    }

    Ref<Model> model = ModelSource::CreateModel(asset, {});
    if (model == nullptr) {
      OE_ERROR("Failed to create model from asset {}", path);
      return 0;
    }

    return model->handle;
  }

  MeshSnapshotter::MeshSnapshotter() {
    AddReaderWriterForField<AssetHandle, 0>(
      [=](ByteBuffer& buffer, const Mesh& mesh) {
        Ref<Model> model = AssetManager::GetAsset<Model>(mesh.handle);
        if (model == nullptr) {
          OE_ERROR("Failed to retrieve model for mesh handle {}", mesh.handle);
          return;
        }

        Ref<ModelSource> source = model->GetModelSource();
        if (source == nullptr) {
          OE_ERROR("Failed to retrieve model source for model {}", model->handle);
          return;
        }

        Path path = Path(*source->GetFileHandle());
        char raw_str[kMaxStringLength];
        std::memset(raw_str, 0, kMaxStringLength);
        std::strncpy(raw_str, path.string().c_str(), path.string().size());
        buffer.BufferBytes(raw_str, kMaxStringLength);
      },
      [=](ByteBuffer& buffer, size_t& offset, Mesh& mesh) {
        std::string path = buffer.ReadStr(offset);
        if (path.empty()) {
          OE_ERROR("Failed to read path from buffer");
          mesh.handle = 0;
          return;
        }

        mesh.handle = GetMeshHandle(path);
        offset += sizeof(kMaxStringLength);
      }
    );
    AddField<UUID, 1>(&Mesh::material);
    // AddField<std::vector<UUID>, 2>(&Mesh::bone_entity_ids);
    AddField<bool, 2>(&Mesh::visible);
  }

  StaticMeshSnapshotter::StaticMeshSnapshotter() {
    AddField<UUID, 0>(&StaticMesh::material);
    AddField<bool, 1>(&StaticMesh::visible);
    AddField<bool, 2>(&StaticMesh::is_primitive);
    AddField<uint32_t, 3>(&StaticMesh::primitive_id);
    AddField<uint32_t, 4>(&StaticMesh::primitive_selection);
  }

  void MeshSerializer::Serialize(std::ostream& stream, Entity* entity, const Ref<Scene>& scene) const {
    SerializeComponentSection(stream, entity, "mesh");
    /// need to retrieve the path from the asset handler, this is temporary
    auto& mesh = entity->GetComponent<Mesh>();
    SerializeValue(stream, "handle", mesh.handle);
    /// serialize material table handle/data
    SerializeList(stream, "bone-entity-ids", mesh.bone_entity_ids);
    SerializeValue(stream, "visible", mesh.visible);
  }

  void MeshSerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    std::string key_value = GetComponentSectionKey(entity->Name(), std::string{ kMeshValue });

    auto& mesh = entity->AddComponent<Mesh>();
    mesh.visible = scene_table.GetVal<bool>(key_value, kVisibleValue, false).value_or(false);
    mesh.handle = scene_table.GetVal<uint64_t>(key_value, kHandleValue, false).value_or(0);

    Opt<std::string> path = scene_table.GetVal<std::string>(key_value, kPathValue, false);
    if (path.has_value()) {
      AssetHandle handle = GetMeshHandle(path.value());
      if (handle != 0) {
        mesh.handle = handle;
      }
    }

    // std::string material_key = GetComponentSectionKey(entity->Name(), std::string{ kMaterialValue });
    // std::string albedo_key = GetComponentSectionKey(material_key, "albedo");
    // std::string normal_key = GetComponentSectionKey(material_key, "normal");
    // std::string roughness_key = GetComponentSectionKey(material_key, "roughness");

    // auto albedo = scene_table.GetVal<glm::vec4>(key_value, albedo_key, false).value_or(glm::vec4{ 1.f });
    // auto normal = scene_table.GetVal<glm::vec4>(key_value, normal_key, false).value_or(glm::vec4{ 1.f });
    // auto roughness = scene_table.GetVal<glm::vec4>(key_value, roughness_key, false).value_or(glm::vec4{ 1.f });

    // Ref<MaterialTable> table = scene->GetMaterialTable();
    // OE_ASSERT(table != nullptr, "Failed to retrieve material table from scene");

    // mesh.material = table->RegisterMaterial(albedo, normal, roughness);
    // mesh.primitive_selection = mesh.primitive_id;
    // mesh.material = scene_table.GetVal<Material>(key_value, kMaterialValue, false).value_or(Material{});

    /// material data
    /// paths/other metadata
  }

  void StaticMeshSerializer::Serialize(std::ostream& stream, Entity* entity, const Ref<Scene>& scene) const {
    SerializeComponentSection(stream, entity, "static-mesh");
    /// need to retrieve the path from the asset handler, this is temporary
    auto& mesh = entity->GetComponent<StaticMesh>();
    /// serialize material table handle/data
    SerializeValue(stream, "visible", mesh.visible);
    SerializeValue(stream, "is-primitive", mesh.is_primitive);
    if (mesh.is_primitive) {
      SerializeValue(stream, "primitive", mesh.primitive_id);
    }
  }

  void StaticMeshSerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr, "Attempting to deserialize a static-mesh into null entity or scene!");
    std::string key_value = GetComponentSectionKey(entity->Name(), std::string{ kStaticMeshValue });

    auto& mesh = entity->AddComponent<StaticMesh>();

    std::string albedo_key = GetComponentSectionKey(std::string{ kMaterialValue }, "albedo");
    std::string normal_key = GetComponentSectionKey(std::string{ kMaterialValue }, "normal");
    std::string roughness_key = GetComponentSectionKey(std::string{ kMaterialValue }, "roughness");

    auto albedo = scene_table.GetVal<glm::vec4>(key_value, albedo_key, false);
    auto normal = scene_table.GetVal<glm::vec4>(key_value, normal_key, false);
    auto roughness = scene_table.GetVal<glm::vec4>(key_value, roughness_key, false);

    Ref<MaterialTable> table = AssetManager::GetMaterialTable();
    OE_ASSERT(table != nullptr, "Failed to retrieve material table from scene");

    if (albedo.has_value() && normal.has_value() && roughness.has_value()) {
      OE_DEBUG(" > Mesh material found : albedo = {0}, normal = {1}, roughness = {2}", albedo.value(), normal.value(), roughness.value());
      mesh.material = table->RegisterMaterial(*albedo, *normal, *roughness);
    } else {
      OE_DEBUG(" > Mesh material not found, using default");
      mesh.material = table->DefaultMaterial();
    }

    OE_ASSERT(mesh.material.Get() != 0, "Failed to register material for static mesh");
    OE_ASSERT(table->HasMaterial(mesh.material), "Material not found in table");
    mesh.primitive_selection = mesh.primitive_id;

    /// we dont deserialize the handle because CreateBox below will create a new one,
    ///   because the old would be invalid anyways
    mesh.visible = scene_table.GetVal<bool>(key_value, kVisibleValue, false).value_or(true);
    mesh.primitive_id = scene_table.GetVal<uint32_t>(key_value, kPrimitiveValue, false).value_or(0);
    mesh.is_primitive = scene_table.GetVal<bool>(key_value, kIsPrimitiveValue, false).value_or(true);
    mesh.is_primitive == mesh.primitive_id != 0;

    if (!mesh.is_primitive) {
      OE_DEBUG("Unimplented deserialization of non-primitive static mesh for {} : {} ({})", key_value, mesh.primitive_id, mesh.is_primitive);
      return;
    }
    OE_DEBUG("Creating {} with primitive : {}", key_value, mesh.primitive_id);

    /// will be replaced during scene update
    auto& transform = entity->GetComponent<Transform>();
    switch (mesh.primitive_id) {
      case kTriangleIdx: {
        mesh.handle = ModelFactory::CreateTriangle();
      } break;
      case kRectIdx: {
        mesh.handle = ModelFactory::CreateRect();
      } break;

      case kCubeIdx: {
        mesh.handle = ModelFactory::CreateBox();
      } break;

      case kSphereIdx:
        mesh.handle = ModelFactory::CreateSphere(transform.scale.x / 2.f);
        break;

      case kCapsuleIdx:
        mesh.handle = ModelFactory::CreateCapsule(transform.scale.x / 2.f, transform.scale.y);
        break;

      default:
        if (mesh.is_primitive) {
          OE_WARN("Unimplemented or invalid collider shape type : {}", mesh.primitive_id);
        } else {
          OE_ERROR("Corrupted primitive id deserializing {}", key_value);
        }
        break;
    }

    mesh.primitive_selection = mesh.primitive_id;
    OE_DEBUG("Deserialized static mesh {} : {}", key_value, mesh.primitive_id);
  }

}  // namespace other
