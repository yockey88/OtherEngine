
/**
 * \file physics/3D/physics_world.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_WORLD_HPP
#define OTHER_ENGINE_PHYSICS_WORLD_HPP

#include <map>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/time.hpp"
#include "core/uuid.hpp"

#include "ecs/components/transform.hpp"

#include "physics/3D/physics_body.hpp"
#include "physics/3D/physics_shape.hpp"
#include "rendering/scene_renderer.hpp"

namespace other {

  class Scene;

  class PhysicsWorld : public RefCounted {
   public:
    PhysicsWorld(Scene* scene_ctx);
    virtual ~PhysicsWorld();

    static Ref<PhysicsWorld> Create(Scene* scene_ctx);

    virtual void ResetSimulation(Scene* scene) = 0;
    virtual void Simulate(float ts) = 0;
    virtual void CreateBody(Entity& entity) = 0;
    virtual void DestroyBody(Entity& entity) = 0;

    virtual Ref<PhysicsShape> CreateBoxShape(const glm::vec3& half_extents) = 0;
    virtual Ref<PhysicsShape> CreateSphereShape(float radius) = 0;
    virtual Ref<PhysicsShape> CreateCapsuleShape(float radius, float height) = 0;
    virtual Ref<PhysicsShape> CreateConvexMeshShape(const std::vector<float>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) = 0;
    virtual Ref<PhysicsShape> CreateConcaveMeshShape(const std::vector<float>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) = 0;

    virtual void SetDebugRendering(bool debug) = 0;
    bool IsDebugRenderEnabled() const;

    virtual void SubmitDebugRender(Ref<SceneRenderer> renderer) = 0;

    void RegisterColliderShape(UUID entity_id, Ref<PhysicsShape> shape);
    void UnregisterColliderShape(Ref<PhysicsBody> body, Ref<PhysicsShape> shape);

    bool ShouldInterpolateTransform() const;
    float InterpolationAlpha() const;

   protected:
    bool debug_render_enabled = false;

    Opt<time::TimePoint> prev_time = std::nullopt;
    time::TimePoint current_time;
    time::FloatDuration delta_time;

    bool interpolate_physics = false;
    float accumulator = 0.f;
    float alpha = 0.f;

    struct DebugFrameData {
      Ref<Shader> shader = nullptr;
      Ref<VertexArray> physics_triangles_vao = nullptr;
      Ref<VertexArray> physics_lines_vao = nullptr;
    } debug_data;

    std::map<UUID, Ref<PhysicsShape>> shapes[PhysicsShape::Shape::NUM_PHYSICS_SHAPES] = { {} };

    virtual void RegisterCallbacks() {}

    Scene* scene_context = nullptr;

   private:
    std::map<void*, UUID> native_bodies;
    std::map<UUID, Ref<PhysicsBody>> bodies;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PHYSICS_WORLD_HPP
