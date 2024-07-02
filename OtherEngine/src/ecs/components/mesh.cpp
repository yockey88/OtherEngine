/**
 * \file ecs/comopnents/mesh.cpp
 **/
#include "ecs/components/mesh.hpp"

#include "core/config_keys.hpp"

#include "ecs/entity.hpp"

namespace other {

  void MeshSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    SerializeComponentSection(stream , entity , "mesh");
    /// need to retrieve the path from the asset handler, this is temporary
    auto& mesh = entity->GetComponent<Mesh>();
    SerializeValue(stream , "handle" , mesh.handle);
    /// serialize material table handle/data
    SerializeList(stream , "bone-entity-ids" , mesh.bone_entity_ids);
    SerializeValue(stream , "visible" , mesh.visible);
  }

  void MeshSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    std::string key_value = GetComponentSectionKey(entity->Name(), std::string{ kMeshValue });

    auto& mesh = entity->AddComponent<Mesh>();
    mesh.visible = scene_table.GetVal<bool>(key_value , kVisibleValue).value_or(false);
    mesh.handle = scene_table.GetVal<uint64_t>(key_value , kHandleValue).value_or(0);

    /// material data
    /// paths/other metadata
  }
  
  void StaticMeshSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    SerializeComponentSection(stream , entity , "mesh");
    /// need to retrieve the path from the asset handler, this is temporary
    auto& mesh = entity->GetComponent<StaticMesh>();
    SerializeValue(stream , "handle" , mesh.handle);
    /// serialize material table handle/data
    SerializeValue(stream , "visible" , mesh.visible);
    SerializeValue(stream , "is-primitive" , mesh.is_primitive);
    if (mesh.is_primitive) {
      SerializeValue(stream , "primitive" , mesh.primitive_id);
    }
  }

  void StaticMeshSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    std::string key_value = GetComponentSectionKey(entity->Name(), std::string{ kStaticMeshValue });

    auto& mesh = entity->AddComponent<StaticMesh>();
    mesh.visible = scene_table.GetVal<bool>(key_value , kVisibleValue).value_or(false);
    mesh.handle = scene_table.GetVal<uint64_t>(key_value , kHandleValue).value_or(0);
    mesh.is_primitive = scene_table.GetVal<bool>(key_value , kIsPrimitiveValue).value_or(false);
    if (mesh.is_primitive) {
      mesh.primitive_id = scene_table.GetVal<uint32_t>(key_value , kPrimaryValue).value_or(0);
    }

    mesh.primitive_selection = mesh.primitive_id;

    /// material data
    /// paths/other metadata
  }

} // namespace other
