/**
 * \file ecs/entity.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_HPP
#define OTHER_ENGINE_ENTITY_HPP

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "core/logger.hpp"
#include "core/ref.hpp"
#include "core/uuid.hpp"

#include "ecs/component.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/serialization_data.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "scene/scene.hpp"

namespace other {

  class Scene;

  class Entity {
   public:
    /// this is allow us to modify components using our api instead of entt's
    ///   and also allows us to create really easy temporary entities in places
    /// NOTE: this will mean the scene context pointer internall is null
    Entity(entt::registry& registry, entt::entity handle)
        : registry(registry), handle(handle) {}

    /// this is for loading entities from file in SceneManager
    Entity(Ref<Scene>& ctx, UUID uuid, const std::string& name);

    ~Entity() {}

    const Ref<Scene> GetContext() const { return context; }

    const entt::entity& Handle() const { return handle; }

    const UUID& GetUUID() const { return uuid; }
    const std::string Name() const { return GetComponent<Tag>().name; }

    operator bool() const { return handle != entt::null; }
    operator entt::entity() const { return handle; }

    template <ComponentType... T>
    inline bool HasComponent() const {
      return registry.try_get<T...>(handle) != nullptr;
    }

    bool CheckForComponentByName(const std::string_view name);

    template <ComponentType T>
    inline T& GetComponent() {
      return registry.get<T>(handle);
    }

    template <ComponentType T>
    inline const T& ReadComponent() const {
      return registry.get<const T>(handle);
    }

    template <ComponentType T>
    inline T GetComponent() const {
      return registry.get<T>(handle);
    }

    void RegisterComponent(Component& c) {
      c.parent_handle = this;
      c.parent_uuid = uuid;
      c.parent_id = handle;
    }

    template <ComponentType T, typename... Args>
    inline T& AddComponent(Args&&... args) {
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

    inline entt::entity GetEntity() const { return handle; }

    inline bool IsNull() const { return handle == entt::null; }
    inline bool IsNotNull() const { return !IsNull(); }

    inline bool IsValid() const { return registry.valid(handle); }
    inline bool IsOrphan() const { return registry.orphan(handle); }

    inline void SetContext(Ref<Scene>& scene) { context = scene; }

    inline bool operator==(const Entity& other) const { return handle == other.handle; }
    inline bool operator!=(const Entity& other) const { return handle != other.handle; }

    /// for marking as visited during tree traversals
    bool visited = false;

   private:
    friend class Scene;

    Ref<Scene> context;

    entt::registry& registry;
    entt::entity handle = entt::null;

    UUID uuid = 0;
    std::string name = "Entity";

    /// this is for the scene to call internally if wants
    Entity(Scene* ctx, UUID uuid, const std::string& name);
  };

}  // namespace other

ECHO_TYPE(
  type(other::Entity),
  func(GetUUID),
  func(Name),
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
