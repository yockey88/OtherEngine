
/**
 * \file physics/3D/physics_shape.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_SHAPE_HPP
#define OTHER_ENGINE_PHYSICS_SHAPE_HPP

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

namespace other {

  enum class PhysicsShapeType {
    BOX = 0,
    SPHERE,
    CAPSULE,
    CONVEX_MESH,
    CONCAVE_MESH,
    // COMPOUND_SHAPE,
    // MUTABLE_COMPOUND_SHAPE,

    NUM_PHYSICS_SHAPES,
    INVALID_PHYSICS_SHAPE = NUM_PHYSICS_SHAPES,
  };

  class PhysicsShape : public RefCounted {
   public:
    virtual ~PhysicsShape() {}

    // virtual void SetCollisionMaterial(const UUID& material) = 0;

    PhysicsShapeType ShapeType() const;

   protected:
    PhysicsShape(PhysicsShapeType type)
        : type(type) {}

   private:
    PhysicsShapeType type;
  };

  class BoxShape : public PhysicsShape {
   public:
    BoxShape()
        : PhysicsShape(PhysicsShapeType::BOX) {}
    virtual ~BoxShape() override {}

    virtual glm::vec2 HalfExtents() const = 0;
  };

  class SphereShape : public PhysicsShape {
   public:
    SphereShape()
        : PhysicsShape(PhysicsShapeType::SPHERE) {}

    virtual float Radius() const = 0;
  };

  class CapsuleShape : public PhysicsShape {
   public:
    CapsuleShape()
        : PhysicsShape(PhysicsShapeType::CAPSULE) {}

    virtual float Radius() const = 0;
    virtual float Height() const = 0;
  };

  class ConvexMeshShape : public PhysicsShape {
   public:
    ConvexMeshShape()
        : PhysicsShape(PhysicsShapeType::CONVEX_MESH) {}
  };

  class ConcaveMeshShape : public PhysicsShape {
   public:
    ConcaveMeshShape()
        : PhysicsShape(PhysicsShapeType::CONCAVE_MESH) {}
  };

  // class CompoundShape : public PhysicsShape {
  //  public:
  //   CompoundShape()
  //       : PhysicsShape(PhysicsShapeType::COMPOUND_SHAPE) {}
  // };

  // class MutableCompoundShape : public PhysicsShape {
  //  public:
  //   MutableCompoundShape()
  //       : PhysicsShape(PhysicsShapeType::MUTABLE_COMPOUND_SHAPE) {}
  // };

}  // namespace other

#endif  // !OTHER_ENGINE_PHYSICS_SHAPE_HPP
