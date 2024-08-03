
/**
 * \file physics/3D/physics_shape.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_SHAPE_HPP
#define OTHER_ENGINE_PHYSICS_SHAPE_HPP

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

namespace other {

  class PhysicsShape {
    public:
      virtual ~PhysicsShape() {}

    private:
  };

  class BoxShape : public PhysicsShape {
    public:
      virtual ~BoxShape() override {}

    private:
      JPH::Ref<JPH::BoxShapeSettings> settings = nullptr;
      JPH::Ref<JPH::RotatedTranslatedShape> shape = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_PHYSICS_SHAPE_HPP
