/**
 * \file ecs/comopnents/mesh.cpp
 **/
#include "ecs/components/mesh.hpp"

#include "ecs/entity.hpp"

namespace other {

  void MeshSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    /// this is going to be tricky  
    stream << "[" << entity->Name() << ".mesh]\n";
  }

  void MeshSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    /// find mesh file
    /// add mesh file into asset manager
    ///     - we will import the mesh lazily later
    /// add mesh component to entity
    auto& mesh = entity->AddComponent<Mesh>();

    /// retrieve apropriate asset handle to entity mesh component
    /// mesh.handle = <correct-asset-handle>
  }

} // namespace other
