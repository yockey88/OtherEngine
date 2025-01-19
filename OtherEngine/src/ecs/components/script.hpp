/**
 * \file ecs/components/script.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_HPP
#define OTHER_ENGINE_SCRIPT_HPP

#include <concepts>
#include <map>
#include <type_traits>

#include <entt/entity/fwd.hpp>

#include "core/uuid.hpp"

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

#include "scripting/cs/cs_object.hpp"
#include "scripting/script_object.hpp"

namespace other {

  struct ScriptObjectData {
    std::string module;
    std::string obj_name;
  };

  struct Script : Component {
    ECS_COMPONENT(Script, kScriptIndex);

    Script(Scene* scene) : Component(kScriptIndex) {}

    UUID AddScript(const std::string_view name, const std::string_view nspace, const std::string_view module);
    void RemoveScript();

    void ApiCall(const std::string_view name);

    template <typename T>
    void ApiCall(const std::string_view name, T&& dt) {
      OE_ASSERT(script_object != nullptr, "Script object is null");
      script_object->CallMethod<void, T>(std::string{ name }, std::forward<T>(dt));
    }

    template <typename T>
      requires std::is_pointer_v<T>
    void ApiCall(const std::string_view name, T ptr) {
      OE_ASSERT(ptr != nullptr, "Pointer is null");
      script_object->CallMethod<void, T>(std::string{ name }, std::forward<T>(ptr));
    }

    template <typename T>
    T GetProperty(const std::string_view name) {
      OE_ASSERT(script_object != nullptr, "Script object is null");
      return script_object->GetProperty<T>(std::string{ name });
    }

    template <typename T>
    void SetProperty(const std::string_view name, T&& arg) {
      OE_ASSERT(script_object != nullptr, "Script object is null");
      script_object->SetProperty<T>(std::string{ name }, std::forward<T>(arg));
    }

    template <typename T>
    T GetField(const std::string_view name) {
      OE_ASSERT(script_object != nullptr, "Script object is null");
      return script_object->GetField<T>(std::string{ name });
    }

    bool ValidateScripts();

    void SetHandles();

   private:
    ScriptObjectData object_data = {};
    ScriptRef<CsObject> script_object = {};
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

}  // namespace other

ECHO_TYPE(
  type(other::Script, refl::attr::bases<other::Component>)
);

#endif  // !OTHER_ENGINE_SCRIPT_HPP
