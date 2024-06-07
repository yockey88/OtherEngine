/**
 * \file ecs/components/relationship.hpp
 **/
#ifndef OTHER_ENGINE_RELATIONSHIP_HPP
#define OTHER_ENGINE_RELATIONSHIP_HPP

#include <vector>

#include "core/defines.hpp"
#include "core/uuid.hpp"
#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Relationship : public Component {
    Opt<UUID> parent;
    std::vector<UUID> children;

    ECS_COMPONENT(Relationship);
  };
  
  class RelationshipSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(Relationship);
  };

} // namespace other 

#endif // !OTHER_ENGINE_RELATIONSHIP_HPP
