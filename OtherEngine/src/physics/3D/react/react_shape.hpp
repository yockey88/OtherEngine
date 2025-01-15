/**
 * \file physics/3D/react/react_shape.hpp
 **/
#ifndef OTHER_ENGINE_REACT_SHAPE_HPP
#define OTHER_ENGINE_REACT_SHAPE_HPP

#include <reactphysics3d/reactphysics3d.h>

#include "physics/3D/physics_shape.hpp"

namespace other {

  class ReactWorld;

  class ReactBoxShape : public BoxShape {
   public:
    ReactBoxShape(rp3d::BoxShape* shape, ReactWorld* world)
        : BoxShape(), shape(shape), world(world) {}
    virtual ~ReactBoxShape() override {}

    virtual glm::vec2 HalfExtents() const override;

    rp3d::BoxShape* shape = nullptr;
    ReactWorld* world = nullptr;
  };

  class ReactSphereShape : public SphereShape {
   public:
    ReactSphereShape(rp3d::SphereShape* shape, ReactWorld* world)
        : SphereShape(), shape(shape), world(world) {}
    virtual ~ReactSphereShape() override {}

    virtual float Radius() const override;

    rp3d::SphereShape* shape = nullptr;
    ReactWorld* world = nullptr;
  };

  class ReactCapsuleShape : public CapsuleShape {
   public:
    ReactCapsuleShape(rp3d::CapsuleShape* shape, ReactWorld* world)
        : CapsuleShape(), shape(shape), world(world) {}
    virtual ~ReactCapsuleShape() override {}

    virtual float Radius() const override;
    virtual float Height() const override;

    rp3d::CapsuleShape* shape = nullptr;
    ReactWorld* world = nullptr;
  };

  class ReactConvexMeshShape : public ConvexMeshShape {
   public:
    ReactConvexMeshShape(rp3d::ConvexShape* shape, ReactWorld* world)
        : ConvexMeshShape(), shape(shape), world(world) {}
    virtual ~ReactConvexMeshShape() override {}

    rp3d::ConvexShape* shape = nullptr;
    ReactWorld* world = nullptr;
  };

  class ReactConcaveMeshShape : public ConcaveMeshShape {
   public:
    ReactConcaveMeshShape(rp3d::ConcaveShape* shape)
        : ConcaveMeshShape(), shape(shape) {}
    virtual ~ReactConcaveMeshShape() override {}

    rp3d::ConcaveShape* shape = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_SHAPE_HPP