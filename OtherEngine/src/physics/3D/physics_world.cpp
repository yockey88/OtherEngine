
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
#define OTHER_USE_REACT
#ifdef OTHER_USE_REACT
    return NewRef<ReactWorld>();
#else
    return NewRef<JoltWorld>();
#endif
    return nullptr;
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
