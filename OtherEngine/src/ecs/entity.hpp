/**
 * \file ecs/entity.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_HPP
#define OTHER_ENGINE_ENTITY_HPP

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "core/logger.hpp"
#include "core/uuid.hpp"
#include "scene/scene.hpp"
#include "ecs/component.hpp"

namespace other {

  struct Octant;
  class Scene;

  class Entity {
    public:
      Entity() = default;
      Entity(Scene* context , entt::entity handle, UUID uuid , const std::string& name)
        : context(context) , handle(handle), uuid(uuid) , name(name) {}

      const Scene* GetContext() const { return context; }

      const entt::entity& Handle() const { return handle; }
      const glm::vec3& Position() const { return position; }

      const UUID& GetUUID() const { return uuid; }
      const std::string& Name() const { return name; }

      operator bool() const { return handle != entt::null; }
      operator entt::entity() const { return handle; }
      
      template <typename T>
      inline bool HasComponent() { return context->registry.try_get<T>(handle) != nullptr; }

      template <typename T>
      inline T& GetComponent() {
        return context->registry.get<T>(handle);
      }

      template <typename T , typename... Args>
      inline T& AddComponent(Args&&... args) {
        if (HasComponent<T>()) {
          OE_WARN("Component already exists on entity {}" , Name()); 
          return GetComponent<T>();
        }

        return context->registry.emplace<T>(handle , std::forward<Args>(args)...);
      }

      template<typename T , typename... Args>
      inline T& AddOrReplace(Args&&... args) {
        return context->registry.emplace_or_replace<T>(handle , std::forward<Args>(args)...);
      }

      template <typename T , typename... Args>
      inline T& ReplaceComponent(Args&&... args) {
        return context->registry.replace<T>(handle , std::forward<Args>(args)...);
      }

      template <typename T>
      inline void RemoveComponent() {
        if (!HasComponent<T>()) {
          OE_WARN("Entity does not have component!");
          return;
        }
        context->registry.remove<T>(handle);
      }

      inline entt::entity GetEntity() const { return handle; }

      inline bool IsNull() const { return handle == entt::null; }
      inline bool IsNotNull() const { return !IsNull(); }

      inline bool IsValid() const { return context->registry.valid(handle); }
      inline bool IsOrphan() const { return context->registry.orphan(handle); }

      inline void SetContext(Scene* scene) { context = scene; }

      inline bool operator==(const Entity& other) const { return handle == other.handle; }
      inline bool operator!=(const Entity& other) const { return handle != other.handle; }

    private:
      Scene* context = nullptr;

      entt::entity handle = entt::null;
      glm::vec3 position = glm::vec3(0.0f);

      UUID uuid = 0;
      std::string name = "Entity";
  };

} // namespace other

#endif // !OTHER_ENGINE_ENTITY_HPP
