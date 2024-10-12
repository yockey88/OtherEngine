/**
 * \file ecs/components/script.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_HPP
#define OTHER_ENGINE_SCRIPT_HPP

#include <entt/entity/fwd.hpp>
#include <map>
#include <type_traits>

#include "core/uuid.hpp"
#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"
#include "scripting/script_object.hpp"
#include "scripting/cs/cs_object.hpp"

namespace other {

  struct ScriptObjectData {
    std::string module;
    std::string obj_name;
  };

  struct Script : Component {
    ECS_COMPONENT(Script , kScriptIndex);

    UUID AddScript(const std::string_view name , const std::string_view nspace , const std::string_view module);
    void RemoveScript(UUID id);
    void RemoveScript(const std::string_view name);

    void ApiCall(const std::string_view name);
    void ApiCall(const std::string_view name , float dt);

    bool ValidateScripts();

    void SetHandles();

    void Clear();

    const auto& GetScripts() const { return scripts; }

    private:
      std::map<UUID , ScriptObjectData> data = {};
      std::map<UUID , ScriptRef<CsObject>> scripts = {};
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
