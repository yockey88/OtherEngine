
/**
 * \file physics/3D/physics_world.cpp
 **/
#include "physics/3D/physics_world.hpp"

#include "scene/scene.hpp"

#include "physics/3D/react/react_world.hpp"

namespace other {

  PhysicsWorld::PhysicsWorld() {}

  PhysicsWorld::~PhysicsWorld() {}

  Ref<PhysicsWorld> PhysicsWorld::Create() {
    Ref<PhysicsWorld> world = nullptr;

#define OTHER_USE_REACT
#ifdef OTHER_USE_REACT
    world = NewRef<ReactWorld>();
#else
    world = NewRef<JoltWorld>();
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
      OE_WARN("Failed to register collider shape for entity: {0}", entity_id);
    }

    OE_INFO("Registered collider shape for entity: {0}", entity_id);
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
