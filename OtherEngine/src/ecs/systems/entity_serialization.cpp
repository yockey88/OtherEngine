/**
 * \file ecs/systems/entity_serialization.cpp
 **/
#include "ecs/systems/entity_serialization.hpp"

#include <array>

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"
#include "ecs/components/transform.hpp"

namespace other {

  constexpr static std::array<Scope<ComponentSerializer>(*)() , kBuiltInComponents> kComponentSerializerTable = {
    /// tags are implicitly deserialized so this should never be called
    []() -> Scope<ComponentSerializer> { return nullptr; } ,
    []() -> Scope<ComponentSerializer> { return NewScope<TransformSerializer>(); } ,
    []() -> Scope<ComponentSerializer> { return nullptr; } ,
  };

  Scope<ComponentSerializer> EntitySerialization::GetComponentSerializer(const std::string_view tag) {
    auto itr = std::find_if(kBuiltInComponentTags.begin() , kBuiltInComponentTags.end() , [&tag](const ComponentTagPair& tag_pair) -> bool {
        if (tag == tag_pair.first) {
          return true;
        }
        return false;
    });

    if (itr == kBuiltInComponentTags.end()) {
      /// find plugin to load component
      return nullptr;
    }

    return kComponentSerializerTable[itr->second](); 
  }

} // namespace other
