/**
 * \file ecs/entity.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_HPP
#define OTHER_ENGINE_ENTITY_HPP

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <hosting/native_object.hpp>
#include <reflection/reflected_object.hpp>

#include "core/logger.hpp"
#include "core/ref.hpp"
#include "core/reflection_attributes.hpp"
#include "core/uuid.hpp"

#include "ecs/component.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/serialization_data.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"

#include "rendering/draw_calls.hpp"

namespace other {

  class Scene;

  class Entity {
    // ECHO_REFLECT();
   public:
    Entity(entt::registry& registry, entt::entity handle);
    Entity(entt::registry& registry, UUID uuid, const std::string& name);

    ~Entity() {}

    const Ref<Scene> GetContext() const;
    entt::entity Handle() const;

    UUID GetUUID() const;
    const std::string& Name() const;

    operator bool() const;
    operator entt::entity() const;

    template <ComponentType... T>
    bool HasComponent() const {
      if (sizeof...(T) == 0) {
        return false;
      }

      return registry.all_of<T...>(handle);
    }

    template <ComponentType... T>
    bool HasAnyComponent() const {
      if (sizeof...(T) == 0) {
        return false;
      }
      return registry.any_of<T...>(handle);
    }

    bool CheckForComponentByName(const std::string_view name);

    template <ComponentType T>
    T& GetComponent() {
      OE_ASSERT(HasComponent<T>(), "Entity does not have component!");
      return registry.get<T>(handle);
    }

    template <ComponentType T>
    const T& ReadComponent() const {
      OE_ASSERT(HasComponent<T>(), "Entity does not have component!");
      return registry.get<const T>(handle);
    }

    template <ComponentType T>
    T GetCopy() const {
      OE_ASSERT(HasComponent<T>(), "Entity does not have component!");
      return T{ registry.get<T>(handle) };
    }

    void RegisterComponent(Component& c) {
      c.parent_handle = this;
      c.parent_uuid = uuid;
      c.parent_id = handle;
    }

    template <ComponentType T, typename... Args>
    T& AddComponent(Args&&... args) {
      if (HasComponent<T>()) {
        OE_WARN("Component already exists on entity {}", Name());
        return GetComponent<T>();
      }

      /// default components, no need to add these to serialization data
      if constexpr (std::is_same_v<T, Tag> || std::is_same_v<T, Transform> ||
                    std::is_same_v<T, Relationship> || std::is_same_v<T, SerializationData>) {
        auto& c = registry.emplace<T>(handle, std::forward<Args>(args)...);
        RegisterComponent(c);
        return c;
      }

      auto comp_idx = T().component_idx;
      auto& sdata = GetComponent<SerializationData>();
      sdata.entity_components.insert(comp_idx);

      if constexpr (std::same_as<T, Script>) {
        auto& script = registry.emplace<T>(handle, std::forward<Args>(args)...);
        OE_DEBUG("Setting NativeHandle to {:p} | EntityHandle to {}:{}", fmt::ptr(this), GetUUID(), (uint32_t)handle);
        RegisterComponent(script);
        return script;
      }

      auto& c = registry.emplace<T>(handle, std::forward<Args>(args)...);
      RegisterComponent(c);
      return c;
    }

    template <ComponentType T, typename... Args>
      requires std::constructible_from<T, Args...>
    inline T& AddOrReplace(Args&&... args) {
      if (!HasComponent<T>()) {
        return AddComponent<T>(std::forward<Args>(args)...);
      } else {
        return registry.replace<T>(handle, std::forward<Args>(args)...);
      }
    }

    template <ComponentType T, typename... Args>
      requires std::constructible_from<T, Args...>
    inline T& ReplaceComponent(Args&&... args) {
      return registry.replace<T>(handle, std::forward<Args>(args)...);
    }

    template <ComponentType T>
    inline void RemoveComponent() {
      if (!HasComponent<T>()) {
        OE_WARN("Entity does not have component!");
        return;
      }
      registry.remove<T>(handle);

      auto comp_idx = T().component_idx;
      auto& sdata = GetComponent<SerializationData>();

      auto itr = std::find(sdata.entity_components.begin(), sdata.entity_components.end(), comp_idx);

      /// just to be safe, should be impossible here
      if (itr == sdata.entity_components.end()) {
        return;
      }

      sdata.entity_components.erase(itr);
    }

    template <ComponentType T>
    inline void UpdateComponent(const T& component) {
      registry.patch<T>(handle, [&](auto& comp) { comp = component; });
    }

    entt::entity GetEntity() const;

    bool IsNull() const;
    bool IsNotNull() const;

    bool IsValid() const;
    bool IsOrphan() const;

    bool operator==(const Entity& other) const;
    bool operator!=(const Entity& other) const;

    bool HasVisibleComponent() const;

    RenderStaticSubmission WireframeSubmission() const;

    /// for marking as visited during tree traversals
    bool visited = false;

   private:
    friend class Scene;
    friend class CaptureStack;

    entt::registry& registry;
    entt::entity handle = entt::null;

    UUID uuid = 0;
    std::string name = "Entity";
  };

}  // namespace other

ECHO_TYPE(
  type(other::Entity),
  func(GetUUID, property("id"), other::Serializable()),
  func(Name, property("name"), other::Serializable()),
  func(HasComponent),
  func(GetComponent),
  func(ReadComponent),
  func(AddComponent),
  func(AddOrReplace),
  func(ReplaceComponent),
  func(RemoveComponent),
  func(UpdateComponent),
  func(GetEntity),
  func(IsNull),
  func(IsValid),
  func(IsOrphan)
);

#endif  // !OTHER_ENGINE_ENTITY_HPP
