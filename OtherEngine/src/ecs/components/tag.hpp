/**
 * \file ecs/components/tag.hpp
 **/
#ifndef OTHER_ENGINE_TAG_HPP
#define OTHER_ENGINE_TAG_HPP

#include "core/defines.hpp"
#include "core/uuid.hpp"
#include "ecs/component.hpp"
#include <entt/entity/fwd.hpp>

namespace other {

  struct Tag : public Component {
    std::string name = "[ Blank Entity ]";
    UUID id = 0;
    entt::entity handle = entt::null;

    Tag(const std::string& name)
      : Component(kTagIndex) , name(name) , id(FNV(name)) {}
    Tag(const std::string& name , UUID id)
      : Component(kTagIndex) , name(name) , id(id) {}

    ECS_COMPONENT(Tag , kTagIndex); 
  };  

} // namespace other

#endif // !OTHER_ENGINE_TAG_HPP
