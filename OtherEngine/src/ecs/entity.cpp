/**
 * \file ecs/entity.cpp
 **/
#include "ecs/entity.hpp"

#include "ecs/component.hpp"
#include "ecs/components/serialization_data.hpp"
#include "ecs/components/tag.hpp"
#include "scene/scene.hpp"

namespace other {

  Entity::Entity(Ref<Scene>& ctx, UUID uuid, const std::string& name)
      : registry(ctx->registry), uuid(uuid), name(name) {
    context = ctx;
    handle = context->registry.create();

    auto& tag = GetComponent<Tag>();
    tag.id = uuid;
    tag.name = name;
  }

  Entity::Entity(Scene* ctx, UUID uuid, const std::string& name)
      : registry(ctx->registry), uuid(uuid), name(name) {
    context = Ref<Scene>(ctx);
    handle = registry.create();

    auto& tag = GetComponent<Tag>();
    tag.id = uuid;
    tag.name = name;
  }

  bool Entity::CheckForComponentByName(const std::string_view name) {
    SerializationData& sdata = GetComponent<SerializationData>();
    auto comp_itr = std::ranges::find_if(kComponentTags, [name](const auto& pair) { return pair.first == name; });
    if (comp_itr == kComponentTags.end()) {
      OE_DEBUG("Component {} not found", name);
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

}  // namespace other
