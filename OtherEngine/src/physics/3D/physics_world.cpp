
/**
 * \file physics/3D/physics_world.cpp
 **/
#include "physics/3D/physics_world.hpp"

// #define OTHER_USE_REACT
#ifdef OTHER_USE_REACT
  #include "physics/3D/react/react_world.hpp"
#else
  #include "physics/3D/jolt/jolt_world.hpp"
#endif

namespace other {

  PhysicsWorld::PhysicsWorld(Scene* scene_ctx)
      : scene_context(scene_ctx) {}

  PhysicsWorld::~PhysicsWorld() {}

  Ref<PhysicsWorld> PhysicsWorld::Create(Scene* scene_ctx) {
    Ref<PhysicsWorld> world = nullptr;

#ifdef OTHER_USE_REACT
    world = NewRef<ReactWorld>(scene_ctx);
#else
    world = NewRef<JoltWorld>(scene_ctx);
#endif

    world->RegisterCallbacks();

    return world;
  }

  void PhysicsWorld::RegisterColliderShape(UUID entity_id, Ref<PhysicsShape> shape) {
    OE_ASSERT(shape != nullptr, "Physics shape is null");
    shape->SetEntity(entity_id);

    PhysicsShape::Shape type = shape->ShapeType();
    OE_ASSERT(type < PhysicsShape::Shape::INVALID_PHYSICS_SHAPE, "Invalid physics shape type");

    auto& shape_map = shapes[type];
    auto [it, inserted] = shape_map.insert({ entity_id, shape });
    if (!inserted) {
      /// already registered, no-op
      return;
    }

    OE_INFO("Registered collider shape for entity: {0}", entity_id);
  }

  void PhysicsWorld::UnregisterColliderShape(Ref<PhysicsBody> body, Ref<PhysicsShape> shape) {
    OE_ASSERT(body != nullptr, "Physics body is null");
    OE_ASSERT(shape != nullptr, "Physics shape is null");

    UUID entity_id = body->GetEntityID();
    auto& shape_map = shapes[shape->ShapeType()];
    auto it = shape_map.find(entity_id);
    if (it == shape_map.end()) {
      return;
    }

    auto [id, shape_ref] = *it;
    OE_INFO("Unregistering collider shape for entity: {0}", id);
    body->RemoveCollider(shape_ref);
    shape_map.erase(it);
  }

  bool PhysicsWorld::IsDebugRenderEnabled() const {
    return debug_render_enabled;
  }

  bool PhysicsWorld::ShouldInterpolateTransform() const {
    return interpolate_physics;
  }

  float PhysicsWorld::InterpolationAlpha() const {
    return alpha;
  }

}  // namespace other
