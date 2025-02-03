/**
 * \file ecs/components/terrain.hpp
 **/
#ifndef OTHER_ENGINE_TERRAIN_COMPONENT_HPP
#define OTHER_ENGINE_TERRAIN_COMPONENT_HPP

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Mesh;
  struct Terrain;

  struct TerrainSnapshotter : public ObjectSerializer<Terrain, 2> {
    TerrainSnapshotter();

    static size_t Stride() {
      return sizeof(glm::ivec2) + sizeof(float);
    }
  };

  struct Terrain : public Component {
    glm::ivec2 size = glm::ivec2(64);
    float scale = 1.0f;

    std::vector<float> heights;
    ECS_COMPONENT(Terrain, TERRAIN_COMPONENT_INDEX);

    static void GenerateHeightMap(Mesh* model, Terrain& instance);
  };

  class TerrainSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(Terrain);
  };

}  // namespace other

ECHO_TYPE(
  type(other::Terrain, refl::attr::bases<other::Component>),
  field(size, echo::serializable_field()),
  field(scale, echo::serializable_field()),
  field(heights)
);

#endif  // !OTHER_ENGINE_TERRAIN_COMPONENT_HPP