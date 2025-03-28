/**
 * \file ecs/components/terrain.cpp
 **/
#include "ecs/components/terrain.hpp"

#include "core/config_keys.hpp"

#include "asset/asset_manager.hpp"

#include "ecs/components/mesh.hpp"
#include "ecs/entity.hpp"

namespace other {

  TerrainSnapshotter::TerrainSnapshotter() {
    AddField<glm::ivec2, 0>(&Terrain::size);
    AddField<float, 1>(&Terrain::scale);
  }

  void Terrain::GenerateHeightMap(Mesh* model, Terrain& instance) {
    OE_ASSERT(model != nullptr, "Model is null");
    OE_ASSERT(model->handle != 0, "Model handle is zero");

    Ref<Model> model_asset = AssetManager::GetAsset<Model>(model->handle);
    OE_ASSERT(model_asset != nullptr, "Model asset is null : {}", model->handle);

    Ref<ModelSource> source = model_asset->GetModelSource();
    OE_ASSERT(source != nullptr, "Model source is null for model : {}", model->handle);

    const MeshBounds& bounds = source->bounds;
    const std::vector<Vertex>& vertices = source->Vertices();

    float x_range = glm::max(glm::ceil(bounds.x_range), 2.f);
    float y_range = glm::max(glm::ceil(bounds.y_range), 2.f);
    uint32_t dim = x_range * y_range;
    OE_ASSERT(dim >= 1.f, "Invalid terrain dimensions : {}x{}", x_range, y_range);

    /// not sure if this is the best way to do this
    {
      instance.size = glm::ivec2(x_range, y_range);
      instance.heights.resize(dim);
    }

    for (uint32_t i = 0; i < dim; ++i) {
      instance.heights[i] = 0.f;  // vertices[i].position.y;
    }

    OE_TRACE("Generated Height Map from model ({}x{}) [{}] : \n{}\n", instance.size.x, instance.size.y, model->handle, fmt::join(instance.heights, ", "));
  }

  void TerrainSerializer::Serialize(std::ostream& stream, Entity* entity, const Ref<Scene>& scene) const {
    SerializeComponentSection(stream, entity, "terrain");
    auto& terrain = entity->GetComponent<Terrain>();
    // SerializeValue(stream, "width", terrain.width);
    // SerializeValue(stream, "height", terrain.height);
    // SerializeValue(stream, "scale", terrain.scale);
    // SerializeList(stream, "heights", terrain.heights);
  }

  void TerrainSerializer::Deserialize(Entity* entity, const ConfigTable& scene_table, Ref<Scene>& scene) const {
    std::string key_value = GetComponentSectionKey(entity->Name(), std::string{ kTerrainValue });

    [[maybe_unused]] auto& terrain = entity->AddComponent<Terrain>();
    // terrain.width = scene_table.GetVal<uint32_t>(key_value, "width", false).value_or(0);
    // terrain.height = scene_table.GetVal<uint32_t>(key_value, "height", false).value_or(0);
    // terrain.scale = scene_table.GetVal<float>(key_value, "scale", false).value_or(1.0f);
    // terrain.heights = scene_table.GetVal<std::vector<float>>(key_value, "heights", false).value_or(std::vector<float>{});
  }

}  // namespace other