/**
 * \file ecs/entity.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_HPP
#define OTHER_ENGINE_ENTITY_HPP

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "core/logger.hpp"
#include "core/uuid.hpp"
#include "core/ref.hpp"
#include "scene/scene.hpp"
#include "ecs/component.hpp"
#include "ecs/components/tag.hpp"

namespace other {

  struct Octant;
  class Scene;

  class Entity {
    public:
      /// this is allow us to modify components using our api instead of entt's
      ///   and also allows us to create really easy temporary entities in places
      /// NOTE: this will mean the scene context pointer internall is null
      Entity(entt::registry& registry , entt::entity handle)
        : registry(registry) , handle(handle) {}

      /// this is for loading entities from file in SceneManager
      Entity(Ref<Scene>& ctx , UUID uuid , const std::string& name);

      ~Entity() {}

      const Ref<Scene> GetContext() const { return context; }

      const entt::entity& Handle() const { return handle; }

      const UUID& GetUUID() const { return uuid; }
      const std::string Name() const { return GetComponent<Tag>().name; }

      operator bool() const { return handle != entt::null; }
      operator entt::entity() const { return handle; }
      
      template <component_type T>
      inline bool HasComponent() const { 
        return registry.try_get<T>(handle) != nullptr; 
      }

      template <component_type T>
      inline T& GetComponent() {
        return registry.get<T>(handle);
      }
      
      template <component_type T>
      inline const T& ReadComponent() const {
        return registry.get<const T>(handle);
      }
      
      template <component_type T>
      inline T GetComponent() const {
        return registry.get<T>(handle);
      }

      template <component_type T , typename... Args>
      inline T& AddComponent(Args&&... args) {
        if (HasComponent<T>()) {
          OE_WARN("Component already exists on entity {}" , Name()); 
          return GetComponent<T>();
        }

        return registry.emplace<T>(handle , std::forward<Args>(args)...);
      }

      template<component_type T , typename... Args>
        requires std::constructible_from<T , Args...>
      inline T& AddOrReplace(Args&&... args) {
        return registry.emplace_or_replace<T>(handle , std::forward<Args>(args)...);
      }

      template<component_type T , typename... Args>
        requires std::constructible_from<T , Args...>
      inline T& ReplaceComponent(Args&&... args) {
        return registry.replace<T>(handle , std::forward<Args>(args)...);
      }

      template <component_type T>
      inline void RemoveComponent() {
        if (!HasComponent<T>()) {
          OE_WARN("Entity does not have component!");
          return;
        }
        registry.remove<T>(handle);
      }

      template <component_type T>
      inline void UpdateComponent(const T& component) {
        if (!HasComponent<T>()) {
          AddComponent<T>();
        } else {
          registry.patch<T>(handle , [&](auto& comp) { comp = component; });
            //.patch<T>(handle , component);
        }
      }

      const std::vector<uint32_t> GetComponentIndices() const;

      inline entt::entity GetEntity() const { return handle; }

      inline bool IsNull() const { return handle == entt::null; }
      inline bool IsNotNull() const { return !IsNull(); }

      inline bool IsValid() const { return registry.valid(handle); }
      inline bool IsOrphan() const { return registry.orphan(handle); }

      inline void SetContext(Ref<Scene>& scene) { context = scene; }

      inline bool operator==(const Entity& other) const { return handle == other.handle; }
      inline bool operator!=(const Entity& other) const { return handle != other.handle; }

    private:
      friend class Scene;

      Ref<Scene> context;

      entt::registry& registry;
      entt::entity handle = entt::null;

      UUID uuid = 0;
      std::string name = "Entity";
      
      /// this is for the scene to call internally if wants
      Entity(Scene* ctx , UUID uuid , const std::string& name);
  };

} // namespace other

#endif // !OTHER_ENGINE_ENTITY_HPP
