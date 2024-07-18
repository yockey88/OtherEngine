/**
 * \file ecs/components/collider_2d.cpp
 **/
#include "ecs/components/collider_2d.hpp"

#include "core/config_keys.hpp"

#include "ecs/entity.hpp"

namespace other {

  void Collider2DSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    auto& collider = entity->GetComponent<Collider2D>();

    SerializeComponentSection(stream , entity , "collider-2D");

    SerializeVec2(stream , "offset" , collider.offset);
    SerializeVec2(stream , "size" , collider.size);
    SerializeValue(stream , "density" , collider.density);
    SerializeValue(stream , "friction" , collider.friction);
  }

  void Collider2DSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize a rigid-body 2D into null entity or scene!");
    std::string key_value = GetComponentSectionKey(entity->Name() , std::string{ kCollider2DValue });

    auto offset_value = scene_table.Get(key_value , kOffsetValue);
    if (offset_value.size() != 2) {
      OE_ERROR("Failed to deserialize collider 2D into entity {}" , entity->Name());
      return;
    }

    auto size_value = scene_table.Get(key_value , kSizeValue);
    if (size_value.size() != 2) {
      OE_ERROR("Failed to deserialize collider 2D into entity {}" , entity->Name());
      return;
    }

    auto& collider = entity->AddComponent<Collider2D>();

    DeserializeVec2(offset_value , collider.offset);
    DeserializeVec2(size_value , collider.size);
    collider.density = scene_table.GetVal<float>(key_value , kDensityValue).value_or(1.f);
    collider.friction = scene_table.GetVal<float>(key_value , kFrictionValue).value_or(1.f);
  }

} // namespace other
