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

namespace other {

  Entity::Entity(entt::registry& registry, entt::entity handle)
      : dotother::NObject(registry.get<Tag>(handle).id.Get()),
        registry(registry), handle(handle) {
  }

  Entity::Entity(entt::registry& registry, UUID uuid, const std::string& name)
      : dotother::NObject(uuid.Get()), registry(registry), uuid(uuid), name(name) {
    handle = registry.create();

    auto& tag = GetComponent<Tag>();
    tag.id = uuid;
    tag.name = name;
  }

  const entt::entity& Entity::Handle() const {
    return handle;
  }

  const UUID& Entity::GetUUID() const {
    return uuid;
  }

  const std::string Entity::Name() const {
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
    if (idx == kTagIndex || idx == kTransformIndex || idx == kRelationshipIndex) {
      return true;
    }

    /// sanity check
    OE_ASSERT(n == name, "Component name mismatch");
    return std::ranges::find_if(sdata.entity_components, [idx](int32_t i) { return i == idx; }) != sdata.entity_components.end();
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

  RenderSubmission Entity::WireframeSubmission() const {
    AssetHandle wireframe = ModelFactory::CreateBoxWireframe();
    Ref<StaticModel> model = AssetManager::GetAsset<StaticModel>(wireframe);
    OE_ASSERT(model != nullptr, "Failed to get wireframe model");

    Material mat(glm::vec4(235.f / 255.f, 132.f / 255.f, 9.f / 255.f, 1.f), 1.f);

    RenderSubmission submission = {
      .model = model,
      .transform = glm::scale(ReadComponent<Transform>().model_transform, glm::vec3(1.03f)),
      .material = mat,
      .render_state = RenderState::FILL,
      .draw_mode = DrawMode::LINES,
    };
    OE_ASSERT(submission.model != nullptr, "Wireframe model is null!");

    /// TODO: rewrite this to take into account the entities mesh if it has one
    ///       - if it has one render a wireframe of the mesh in highlight color
    ///       - if it does not have a mesh render a wireframe of the bounding box in highlight color
    // if (HasVisibleComponent()) {
    // } else {
    // }
    return submission;
  }

}  // namespace other
