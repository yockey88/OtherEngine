
/**
 * \file physics/3D/physics_world.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_WORLD_HPP
#define OTHER_ENGINE_PHYSICS_WORLD_HPP

#include <map>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

#include "ecs/components/transform.hpp"

#include "physics/3D/physics_body.hpp"
#include "physics/3D/physics_shape.hpp"

namespace other {

  class Scene;

  class PhysicsWorld : public RefCounted {
   public:
    PhysicsWorld();
    virtual ~PhysicsWorld();

    virtual void Simulate(float ts) = 0;
    virtual Ref<PhysicsBody> CreateBody(const Transform& initial_transform) = 0;

    virtual Ref<PhysicsShape> CreateBoxShape(const glm::vec3& half_extents) = 0;
    virtual Ref<PhysicsShape> CreateSphereShape(float radius) = 0;
    virtual Ref<PhysicsShape> CreateCapsuleShape(float radius, float height) = 0;
    virtual Ref<PhysicsShape> CreateCylinderShape(float radius, float height) = 0;
    virtual Ref<PhysicsShape> CreateConeShape(float radius, float height) = 0;
    virtual Ref<PhysicsShape> CreateConvexMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) = 0;

    virtual void SetSceneContext(const Ref<Scene>& scene) = 0;

    float alpha = 0.f;

   private:
    std::map<UUID, Ref<PhysicsBody>> bodies;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PHYSICS_WORLD_HPP
