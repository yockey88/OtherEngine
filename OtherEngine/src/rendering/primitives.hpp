/**
 * \file rendering/primitives.hpp
 **/
#ifndef OTHER_ENGINE_PRIMITIVES_HPP
#define OTHER_ENGINE_PRIMITIVES_HPP

#include "math/vecmath.hpp"

namespace other {

  struct Line {
    Point start;
    Point end;
  };

  struct Triangle {
    Point p1;
    Point p2;
    Point p3;
  }; 

  struct Rect {
    Point position;
    glm::vec2 half_extents;
  };

  struct Cube {
    Point position;
    glm::vec3 half_extents;
  };

  struct Circle {
    Point position;
    float radius;
  };

  struct Sphere {
    Point position;
    float radius;
  };

  struct Capsule {
    Point position;
    glm::vec3 normal;
    float radius;
    float height;
  };
  
  struct Cylinder {
    Point position;
    glm::vec3 normal;
    float radius;
    float height;
  };

} // namespace other

#endif // !OTHER_ENGINE_PRIMITIVES_HPP
