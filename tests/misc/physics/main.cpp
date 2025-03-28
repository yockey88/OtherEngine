/**
 * \file physics/main.cpp
 **/
#include <chrono>

#include <reactphysics3d/reactphysics3d.h>

#include "core/formatters.hpp"
#include "core/time.hpp"

namespace rp3d = reactphysics3d;

int main() {
  rp3d::PhysicsCommon physics_common;
  {
    rp3d::decimal accumulator = 0.f;
    float alpha = 0.f;

    rp3d::PhysicsWorld* physics_world = physics_common.createPhysicsWorld();

    rp3d::Vector3 position(0.f, 20.f, 0.f);
    rp3d::Quaternion orientation = rp3d::Quaternion::identity();
    rp3d::Transform transform(position, orientation);
    rp3d::RigidBody* body = physics_world->createRigidBody(transform);
    body->setType(rp3d::BodyType::DYNAMIC);
    body->setIsActive(true);

    // rp3d::Transform inter_transform = body->getTransform();

    // other::time::TimePoint curr_time = other::time::SteadyClock::now();
    other::time::FrameRateEnforcer<60> enforcer;

    // other::time::TimePoint start_time = other::time::SteadyClock::now();

    uint32_t i = 0;
    while (body->getTransform().getPosition().y > 0.f) {
      physics_world->update(enforcer.TimeStep());

      const rp3d::Transform& transform = body->getTransform();
      const rp3d::Vector3& position = transform.getPosition();

      other::println("[{}] : [{},{},{}]", i++, position.x, position.y, position.z);
    }

    // while (body->getTransform().getPosition().y > 0.f) {
    //   other::time::TimePoint new_time = other::time::SteadyClock::now();
    //   float frame_time = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(new_time - curr_time).count();
    //   curr_time = new_time;

    //   accumulator += frame_time;
    //   while (accumulator >= enforcer.TimeStep()) {
    //     physics_world->update(enforcer.TimeStep());
    //     accumulator -= enforcer.TimeStep();
    //   }

    //   alpha = accumulator / enforcer.TimeStep();

    //   const rp3d::Transform& transf = body->getTransform();
    //   const rp3d::Transform& i_transform = rp3d::Transform::interpolateTransforms(inter_transform, transf, alpha);
    //   inter_transform = i_transform;

    //   rp3d::Vector3 position = i_transform.getPosition();
    //   other::println("[{},{},{}]", position.x, position.y, position.z);

    //   enforcer.Enforce();
    // }
    // other::time::TimePoint end_time = other::time::SteadyClock::now();
    // std::chrono::duration<float> elapsed_time = end_time - start_time;
    // other::println("Elapsed time: [{}]s", elapsed_time.count());

    physics_world->destroyRigidBody(body);
    physics_common.destroyPhysicsWorld(physics_world);
  }

  return 0;
}
