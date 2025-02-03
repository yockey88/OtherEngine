/**
 * \file ecs/entity.cpp
 **/
#include "ecs/entity.hpp"

#include <glm/fwd.hpp>

#include "core/logger.hpp"

#include "asset/asset_manager.hpp"

#include "ecs/component.hpp"
#include "ecs/components/serialization_data.hpp"
#include "ecs/components/tag.hpp"
#include "scene/scene.hpp"

#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"

#include "component.hpp"

namespace other {

  Entity::Entity(entt::registry& registry, entt::entity handle)
      : registry(registry), handle(handle) {
  }

  Entity::Entity(entt::registry& registry, UUID uuid, const std::string& name)
      : registry(registry), uuid(uuid), name(name) {
    handle = registry.create();

    auto& tag = GetComponent<Tag>();
    tag.id = uuid;
    tag.name = name;
  }

  entt::entity Entity::Handle() const {
    return handle;
  }

  UUID Entity::GetUUID() const {
    return uuid;
  }

  const std::string& Entity::Name() const {
    return ReadComponent<Tag>().name;
  }

  Entity::operator bool() const {
    return handle != entt::null;
  }

  Entity::operator entt::entity() const {
    return handle;
  }

  bool Entity::CheckForComponentByName(const std::string_view name) {
    SerializationData& sdata = GetComponent<SerializationData>();
    auto comp_itr = std::ranges::find_if(kComponentTags, [name](const auto& pair) { return pair.name == name; });
    if (comp_itr == kComponentTags.end()) {
      OE_WARN("Component {} not found", name);
      return false;
    }
    auto& [n, idx] = *comp_itr;

    // special case for these components
    if (idx == TAG_COMPONENT_INDEX || idx == TRANSFORM_COMPONENT_INDEX || idx == RELATIONSHIP_COMPONENT_INDEX) {
      return true;
    }

    /// sanity check
    OE_ASSERT(n == name, "Component name mismatch");
    return std::ranges::find_if(sdata.entity_components, [idx](int32_t i) { return i == idx; }) != sdata.entity_components.end();
  }

  Component* Entity::GetComponentByName(const std::string_view name) {
    SerializationData& sdata = GetComponent<SerializationData>();
    auto comp_itr = std::ranges::find_if(kComponentTags, [name](const auto& pair) { return pair.name == name; });
    if (comp_itr == kComponentTags.end()) {
      OE_WARN("Component {} not found", name);
      return nullptr;
    }
    auto& [n, idx] = *comp_itr;

    OE_ASSERT(idx < NUM_COMPONENTS, "Component index out of bounds : {}", idx);
    return GetComponentByIndex(idx);
  }

  Component* Entity::GetComponentByIndex(int32_t idx) {
    OE_ASSERT(idx < NUM_COMPONENTS, "Component index out of bounds : {}", idx);
    auto& metatable = GetComponent<EntityMetatable>();
    return metatable.components[idx];
  }

  entt::entity Entity::GetEntity() const {
    return handle;
  }

  bool Entity::IsNull() const {
    return handle == entt::null;
  }

  bool Entity::IsNotNull() const {
    return !IsNull();
  }

  bool Entity::IsValid() const {
    return registry.valid(handle);
  }

  bool Entity::IsOrphan() const {
    return registry.orphan(handle);
  }

  bool Entity::operator==(const Entity& other) const {
    return handle == other.handle;
  }

  bool Entity::operator!=(const Entity& other) const {
    return handle != other.handle;
  }

  bool Entity::HasVisibleComponent() const {
    return HasComponent<StaticMesh>() || HasComponent<Mesh>();
  }

  RenderStaticSubmission Entity::WireframeSubmission() const {
    AssetHandle wireframe = ModelFactory::CreateBoxWireframe();
    Ref<StaticModel> model = AssetManager::GetAsset<StaticModel>(wireframe);
    OE_ASSERT(model != nullptr, "Failed to get wireframe model");

    Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();

    RenderStaticSubmission submission = {
      .model = model,
      .transform = glm::scale(ReadComponent<Transform>().model_transform, glm::vec3(1.03f)),
      .material_table = material_table,
      .material = material_table->SelectionWireframeMaterial(),
      .draw_mode = DrawMode::LINES,
      .line_thickness = 3.f,
    };
    OE_ASSERT(submission.model != nullptr, "Wireframe model is null!");
    return submission;
  }

}  // namespace other
