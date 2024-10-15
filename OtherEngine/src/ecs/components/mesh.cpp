/**mesh.c
 * \file ecs/comopnents/mesh.cpp
 **/
#include "ecs/components/mesh.hpp"

#include <glm/ext/matrix_transform.hpp>

#include "core/config_keys.hpp"

#include "ecs/entity.hpp"

#include "rendering/model_factory.hpp"

namespace other {

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
    /// we dont deserialize the handle because CreateBox below will create a new one,
    ///   because the old would be invalid anyways
    mesh.visible = scene_table.GetVal<bool>(key_value, kVisibleValue, false).value_or(false);
    mesh.is_primitive = scene_table.GetVal<bool>(key_value, kIsPrimitiveValue, false).value_or(false);
    if (mesh.is_primitive) {
      mesh.primitive_id = scene_table.GetVal<uint32_t>(key_value, kPrimitiveValue, false).value_or(0);
    } else {
      return;
    }

    mesh.primitive_selection = mesh.primitive_id;

    OE_DEBUG("Deserialized mesh {} {}", key_value, mesh.primitive_id);

    if (mesh.primitive_id == 0) {
      return;
    }

    /// will be replaced during scene update
    auto scale = glm::vec3(1.f);
    switch (mesh.primitive_id) {
      case kTriangleIdx: {
        mesh.handle = ModelFactory::CreateTriangle({ scale.x / 2, scale.y / 2 });
      } break;
      case kRectIdx: {
        mesh.handle = ModelFactory::CreateRect({ scale.x / 2, scale.y / 2 });
      } break;

      case kCubeIdx: {
        mesh.handle = ModelFactory::CreateBox();
      } break;

      case kSphereIdx:
      case kCapsuleIdx:
        // Capsule
        break;

      default:
        OE_ERROR("Corrupted primitive id deserializing {}", key_value);
        mesh.primitive_id = 0;
        break;
    }

    /// material data
    /// paths/other metadata
  }

}  // namespace other
