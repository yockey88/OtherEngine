/**
 * \file physics/2D/physics_body_2d.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_BODY_2D_HPP
#define OTHER_ENGINE_PHYSICS_BODY_2D_HPP

#include <box2d/b2_body.h>
#include <box2d/b2_world.h>

#include "core/defines.hpp"
#include "core/ref_counted.hpp"

namespace other {

  class PhysicsBody2D : public RefCounted {
    public:
      PhysicsBody2D(Scope<b2World>& context , const b2BodyDef* body_def);
      ~PhysicsBody2D();

    private:
      Scope<b2World>& context;
      b2Body* body = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_PHYSICS_BODY_2D_HPP
