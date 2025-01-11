/**
 * \file physics/3D/react/react_shape.hpp
 **/
#ifndef OTHER_ENGINE_REACT_SHAPE_HPP
#define OTHER_ENGINE_REACT_SHAPE_HPP

#include <reactphysics3d/reactphysics3d.h>

#include "physics/3D/physics_shape.hpp"

namespace other {

  class ReactBoxShape : public BoxShape {
   public:
    ReactBoxShape(rp3d::BoxShape* shape)
        : BoxShape(), shape(shape) {}
    virtual ~ReactBoxShape() override {}

    virtual glm::vec2 HalfExtents() const override;

    rp3d::BoxShape* shape = nullptr;
  };

  class ReactSphereShape : public SphereShape {
   public:
    ReactSphereShape(rp3d::SphereShape* shape)
        : SphereShape(), shape(shape) {}
    virtual ~ReactSphereShape() override {}

    virtual float Radius() const override;

    rp3d::SphereShape* shape = nullptr;
  };

  class ReactCapsuleShape : public CapsuleShape {
   public:
    ReactCapsuleShape(rp3d::CapsuleShape* shape)
        : CapsuleShape(), shape(shape) {}
    virtual ~ReactCapsuleShape() override {}

    virtual float Radius() const override;
    virtual float Height() const override;

    rp3d::CapsuleShape* shape = nullptr;
  };

  class ReactConvexMeshShape : public ConvexMeshShape {
   public:
    ReactConvexMeshShape(rp3d::ConvexShape* shape)
        : ConvexMeshShape(), shape(shape) {}
    virtual ~ReactConvexMeshShape() override {}

    rp3d::ConvexShape* shape = nullptr;
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