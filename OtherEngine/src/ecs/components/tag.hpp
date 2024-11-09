/**
 * \file ecs/components/tag.hpp
 **/
#ifndef OTHER_ENGINE_TAG_HPP
#define OTHER_ENGINE_TAG_HPP

#include <entt/entity/fwd.hpp>

#include "core/defines.hpp"
#include "core/uuid.hpp"

#include "ecs/component.hpp"

namespace other {

  struct Tag : public Component {
    std::string name = "[ Blank Entity ]";
    UUID id = 0;
    entt::entity handle = entt::null;

    uint64_t GetID() const { return id.Get(); }

    Tag(const std::string& name)
        : Component(kTagIndex), name(name), id(FNV(name)) {}
    Tag(const std::string& name, UUID id)
        : Component(kTagIndex), name(name), id(id) {}

    ECS_COMPONENT(Tag, kTagIndex);
  };

}  // namespace other

ECHO_TYPE(
  type(other::Tag, refl::attr::bases<other::Component>),
  func(GetID, property("id")),
  field(name)
);

#endif  // !OTHER_ENGINE_TAG_HPP
