/**
 * \file ecs/components/position.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENTS_POSITION_HPP
#define OTHER_ENGINE_COMPONENTS_POSITION_HPP

#include <glm/glm.hpp>

#include "ecs/component.hpp"

namespace other {

  struct Position : public Component {
    glm::vec3 position = glm::vec3(0.0f);

    Position() = default;
    Position(const glm::vec3& position) 
      : position(position) {}
    Position(float p) 
      : position(glm::vec3(p)) {}
    Position(float x, float y, float z) 
      : position(glm::vec3(x, y, z)) {}

    template <std::integral T>
    Position(T p) 
      : position(glm::vec3(p)) {}

    template <std::integral T>
    Position(T x, T y, T z) 
      : position(glm::vec3(x, y, z)) {}
  };

} // namespace other

#endif // !OTHER_ENGINE_COMPONENTS_POSITION_HPP
