/**
 * \file physics/3D/jolt/jolt_shape.hpp
 **/
#ifndef OTHER_ENGINE_JOLT_SHAPE_HPP
#define OTHER_ENGINE_JOLT_SHAPE_HPP

// clang-format off
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
// clang-format on

#include "physics/3D/physics_shape.hpp"

namespace other {

  class JoltWorld;

  class JoltBoxShape : public BoxShape {
   public:
    JoltBoxShape(JPH::Ref<JPH::BoxShapeSettings> shape, JPH::Ref<JPH::RotatedTranslatedShape> rt_shape, JoltWorld* world)
        : BoxShape(), shape(shape), rotated_translated_shape(rt_shape), world(world) {}
    virtual ~JoltBoxShape() override {}

    void OnSetEntity(const UUID& id) override;
    void SetTransform(const Transform& transform) override;

    glm::vec2 HalfExtents() const override;
    void* NativeShape() override { return rotated_translated_shape.GetPtr(); }

   private:
    JPH::Ref<JPH::BoxShapeSettings> shape;
    JPH::Ref<JPH::RotatedTranslatedShape> rotated_translated_shape;

    JoltWorld* world;
  };

  class JoltSphereShape : public SphereShape {
   public:
    JoltSphereShape(JPH::Ref<JPH::SphereShapeSettings> shape, JPH::Ref<JPH::RotatedTranslatedShape> rt_shape, JoltWorld* world)
        : SphereShape(), shape(shape), rotated_translated_shape(rt_shape), world(world) {}
    virtual ~JoltSphereShape() override {}

    void OnSetEntity(const UUID& id) override;
    void SetTransform(const Transform& transform) override;

    float Radius() const override;
    void* NativeShape() override { return rotated_translated_shape.GetPtr(); }

   private:
    JPH::Ref<JPH::SphereShapeSettings> shape;
    JPH::Ref<JPH::RotatedTranslatedShape> rotated_translated_shape;

    JoltWorld* world;
  };

  class JoltCapsuleShape : public CapsuleShape {
   public:
    JoltCapsuleShape(JPH::Ref<JPH::CapsuleShapeSettings> shape, JPH::Ref<JPH::RotatedTranslatedShape> rt_shape, JoltWorld* world)
        : CapsuleShape(), shape(shape), rotated_translated_shape(rt_shape), world(world) {}
    virtual ~JoltCapsuleShape() override {}

    void OnSetEntity(const UUID& id) override;
    void SetTransform(const Transform& transform) override;

    float Radius() const override;
    float Height() const override;
    void* NativeShape() override { return rotated_translated_shape.GetPtr(); }

   private:
    JPH::Ref<JPH::CapsuleShapeSettings> shape;
    JPH::Ref<JPH::RotatedTranslatedShape> rotated_translated_shape;

    JoltWorld* world;
  };

  class JoltConvexMeshShape : public ConvexMeshShape {
   public:
    JoltConvexMeshShape(JPH::Ref<JPH::ConvexShapeSettings> shape, JPH::Ref<JPH::RotatedTranslatedShape> rt_shape, JoltWorld* world)
        : ConvexMeshShape(), shape(shape), rotated_translated_shape(rt_shape), world(world) {}
    virtual ~JoltConvexMeshShape() override {}

    void OnSetEntity(const UUID& id) override;
    void SetTransform(const Transform& transform) override;

    void* NativeShape() override { return rotated_translated_shape.GetPtr(); }

   private:
    JPH::Ref<JPH::ConvexShapeSettings> shape;
    JPH::Ref<JPH::RotatedTranslatedShape> rotated_translated_shape;

    JoltWorld* world;
  };

  class JoltConcaveMeshShape : public ConcaveMeshShape {
   public:
    JoltConcaveMeshShape(JPH::Ref<JPH::StaticCompoundShape> shape, JoltWorld* world)
        : ConcaveMeshShape(), shape(shape), world(world) {}
    virtual ~JoltConcaveMeshShape() override {}

    void OnSetEntity(const UUID& id) override;
    void SetTransform(const Transform& transform) override;

    void* NativeShape() override { return shape.GetPtr(); }

   private:
    JPH::Ref<JPH::StaticCompoundShape> shape;

    JoltWorld* world;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_JOLT_SHAPE_HPP