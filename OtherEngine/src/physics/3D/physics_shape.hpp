
/**
 * \file physics/3D/physics_shape.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_SHAPE_HPP
#define OTHER_ENGINE_PHYSICS_SHAPE_HPP

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

#include "ecs/components/transform.hpp"

namespace other {

  class PhysicsShape : public RefCounted {
   public:
    enum Shape {
      BOX = 0,
      SPHERE,
      CAPSULE,
      CONVEX_MESH,
      CONCAVE_MESH,
      COMPOUND_SHAPE,
      // MUTABLE_COMPOUND_SHAPE,
      TERRAIN_SHAPE,

      NUM_PHYSICS_SHAPES,
      INVALID_PHYSICS_SHAPE = NUM_PHYSICS_SHAPES,
    };

    virtual ~PhysicsShape() {}

    // virtual void SetCollisionMaterial(const UUID& material) = 0;
    virtual void* NativeShape() = 0;
    virtual void OnSetEntity(const UUID& id) = 0;
    virtual void SetTransform(const Transform& transform) = 0;
    virtual void SetScale(const glm::vec3& scale) = 0;

    void SetEntity(const UUID& id);

    PhysicsShape::Shape ShapeType() const;

   protected:
    PhysicsShape(PhysicsShape::Shape type)
        : type(type) {}

   private:
    PhysicsShape::Shape type;

    UUID entity_id;
  };

  class BoxShape : public PhysicsShape {
   public:
    BoxShape()
        : PhysicsShape(PhysicsShape::Shape::BOX) {}
    virtual ~BoxShape() override {}

    virtual glm::vec2 HalfExtents() const = 0;
  };

  class SphereShape : public PhysicsShape {
   public:
    SphereShape()
        : PhysicsShape(PhysicsShape::Shape::SPHERE) {}

    virtual float Radius() const = 0;
  };

  class CapsuleShape : public PhysicsShape {
   public:
    CapsuleShape()
        : PhysicsShape(PhysicsShape::Shape::CAPSULE) {}

    virtual float Radius() const = 0;
    virtual float Height() const = 0;
  };

  class ConvexMeshShape : public PhysicsShape {
   public:
    ConvexMeshShape()
        : PhysicsShape(PhysicsShape::Shape::CONVEX_MESH) {}
  };

  class ConcaveMeshShape : public PhysicsShape {
   public:
    ConcaveMeshShape()
        : PhysicsShape(PhysicsShape::Shape::CONCAVE_MESH) {}
  };

  class CompoundShape : public PhysicsShape {
   public:
    CompoundShape()
        : PhysicsShape(PhysicsShape::Shape::COMPOUND_SHAPE) {}
  };

  // class MutableCompoundShape : public PhysicsShape {
  //  public:
  //   MutableCompoundShape()
  //       : PhysicsShape(PhysicsShape::Shape::MUTABLE_COMPOUND_SHAPE) {}
  // };

  class TerrainShape : public PhysicsShape {
   public:
    TerrainShape()
        : PhysicsShape(PhysicsShape::Shape::TERRAIN_SHAPE) {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PHYSICS_SHAPE_HPP
