/**
 * \file ecs/components/script.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_HPP
#define OTHER_ENGINE_SCRIPT_HPP

#include <map>
#include <type_traits>

#include "core/uuid.hpp"
#include "scripting/script_object.hpp"
#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct ScriptObjectData {
    std::string module;
    std::string obj_name;
  };

  struct Script : public Component {
    std::map<UUID , ScriptObjectData> data = {};
    std::map<UUID , ScriptObject*> scripts = {};

    ECS_COMPONENT(Script , kScriptIndex);
  };

  template <typename T>
  concept script_obj_t = std::is_base_of_v<ScriptObject, T>;

  class ScriptSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(Script);
  };

  template <script_obj_t SO>
  SO* ScriptObjectCast(ScriptObject* obj) {
    return static_cast<SO*>(obj);
  }

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_HPP
