/**
 * \file ecs/components/mesh.hpp
 **/
#ifndef OTHER_ENGINE_MESH_HPP
#define OTHER_ENGINE_MESH_HPP

#include "asset/asset_types.hpp"
#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Mesh : public Component {
    AssetHandle handle; 

    ECS_COMPONENT(Mesh);
  }; 

  class MeshSerializer : public ComponentSerializer {
    public:
      virtual ~MeshSerializer() override {}

      COMPONENT_SERIALIZERS(Mesh);
  };

} // namespace other

#endif // !OTHER_ENGINE_MESH_HPP
