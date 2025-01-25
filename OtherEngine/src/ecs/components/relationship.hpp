/**
 * \file ecs/components/relationship.hpp
 **/
#ifndef OTHER_ENGINE_RELATIONSHIP_HPP
#define OTHER_ENGINE_RELATIONSHIP_HPP

#include <set>

#include "core/defines.hpp"
#include "core/uuid.hpp"

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Relationship : public Component {
    Opt<UUID> parent;
    std::set<UUID> children;

    ECS_COMPONENT(Relationship, RELATIONSHIP_COMPONENT_INDEX);
  };

  class RelationshipSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(Relationship);
  };

}  // namespace other

ECHO_TYPE(
  type(other::Relationship, refl::attr::bases<other::Component>)
);

#endif  // !OTHER_ENGINE_RELATIONSHIP_HPP
