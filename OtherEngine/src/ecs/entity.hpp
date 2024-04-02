/**
 * \file ecs/entity.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_HPP
#define OTHER_ENGINE_ENTITY_HPP

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "core/uuid.hpp"
#include "ecs/component.hpp"

namespace other {

  struct Octant;
  class Scene;

  class Entity {
    public:
      Entity() = default;
      Entity(entt::registry* context , entt::entity handle, UUID uuid , const std::string& name)
        : context(context) , handle(handle), uuid(uuid) , name(name) {}

      void PlaceInSpace(Octant* space); 

      Octant* ContainingSpace() const { return containing_space; }

      const entt::entity& Handle() const { return handle; }
      const glm::vec3& Position() const { return position; }

      const UUID& GetUUID() const { return uuid; }
      const std::string& Name() const { return name; }

      operator bool() const { return handle != entt::null; }
      operator entt::entity() const { return handle; }

    private:
      Octant* containing_space = nullptr;

      entt::registry* context = nullptr;

      entt::entity handle = entt::null;
      glm::vec3 position = glm::vec3(0.0f);

      UUID uuid = 0;
      std::string name = "Entity";
  };

} // namespace other

#endif // !OTHER_ENGINE_ENTITY_HPP
