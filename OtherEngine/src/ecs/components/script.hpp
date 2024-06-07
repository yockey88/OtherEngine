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

    private:
      void LoadScripts(Entity* entity , const std::string_view mod_name , const std::string_view mod_path ,
                       const std::vector<std::string>& scripts) const;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_HPP
