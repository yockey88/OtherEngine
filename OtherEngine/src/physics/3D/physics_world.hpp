
/**
 * \file physics/3D/physics_world.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_WORLD_HPP
#define OTHER_ENGINE_PHYSICS_WORLD_HPP

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "core/defines.hpp"
#include "core/ref_counted.hpp"

#include "physics/3D/broad_phase_layer_handler.hpp"
#include "physics/3D/broad_phase_filter.hpp"
#include "physics/3D/object_layer_filter.hpp"

namespace other {

  class PhysicsWorld : public RefCounted {
    public:
      PhysicsWorld();
      ~PhysicsWorld();

      void Simulate(float ts);

      JPH::BodyInterface& GetPhysicsBodies();

    private:
      Scope<JPH::TempAllocatorImpl> temp_alloc = nullptr;
      Scope<JPH::JobSystemThreadPool> thread_pool = nullptr;

      BroadPhaseLayerHandler broad_phase_layer_handler;
      BroadPhaseLayerFilter broad_phase_layer_filter;
      ObjectLayerFilter obj_layer_filter;

      Scope<JPH::PhysicsSystem> system = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_PHYSICS_WORLD_HPP
