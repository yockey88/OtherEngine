/**
 * \file ecs/components/script.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_HPP
#define OTHER_ENGINE_SCRIPT_HPP

#include <map>

#include "core/uuid.hpp"
#include "scripting/script_object.hpp"
#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Script : public Component {
    std::map<UUID , ScriptObject*> scripts = {};

    ECS_COMPONENT(Script);
  };

  class ScriptSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(Script);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_HPP
