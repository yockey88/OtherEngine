/**
 * \file ecs/systems/entity_serialization.cpp
 **/
#include "ecs/systems/entity_serialization.hpp"

#include <array>

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/camera.hpp"

namespace other {

  constexpr static std::array<Scope<ComponentSerializer>(*)() , kNumComponents> kComponentSerializerTable = {
    /// tags are implicitly deserialized so this should never be called
    []() -> Scope<ComponentSerializer> { return nullptr; } ,
    []() -> Scope<ComponentSerializer> { return NewScope<TransformSerializer>(); } ,
    []() -> Scope<ComponentSerializer> { return NewScope<RelationshipSerializer>(); } ,
    []() -> Scope<ComponentSerializer> { return NewScope<MeshSerializer>(); } ,
    []() -> Scope<ComponentSerializer> { return NewScope<ScriptSerializer>(); } ,
    []() -> Scope<ComponentSerializer> { return NewScope<CameraSerializer>(); } , 
    []() -> Scope<ComponentSerializer> { return NewScope<RigidBody2DSerializer>(); } ,
  };

  Scope<ComponentSerializer> EntitySerialization::GetComponentSerializer(const std::string_view tag) {
    auto itr = std::find_if(kComponentTags.begin() , kComponentTags.end() , [&tag](const ComponentTagPair& tag_pair) -> bool {
        if (tag == tag_pair.first) {
          return true;
        }
        return false;
    });

    if (itr == kComponentTags.end()) {
      /// find plugin to load component
      return nullptr;
    }

    return kComponentSerializerTable[itr->second](); 
  }
      
  Scope<ComponentSerializer> EntitySerialization::GetComponentSerializer(uint32_t idx) {
    auto itr = std::find_if(kComponentTags.begin() , kComponentTags.end() , [&idx](const ComponentTagPair& tag_pair) -> bool {
        if (idx == tag_pair.second) {
          return true;
        }
        return false;
    });

    if (itr == kComponentTags.end()) {
      /// find plugin to load component
      return nullptr;
    }

    return kComponentSerializerTable[itr->second](); 
  }

} // namespace other
