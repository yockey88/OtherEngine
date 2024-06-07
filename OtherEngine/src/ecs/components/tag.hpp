/**
 * \file ecs/components/tag.hpp
 **/
#ifndef OTHER_ENGINE_TAG_HPP
#define OTHER_ENGINE_TAG_HPP

#include "core/defines.hpp"
#include "core/uuid.hpp"
#include "ecs/component.hpp"

namespace other {

  struct Tag : public Component {
    std::string name = "[ Blank Entity ]";
    UUID id = 0;

    Tag(const std::string& name)
      : name(name) , id(FNV(name)) {}
    Tag(const std::string& name , UUID id)
      : name(name) , id(id) {}

    ECS_COMPONENT(Tag); 
  };  

} // namespace other

#endif // !OTHER_ENGINE_TAG_HPP
