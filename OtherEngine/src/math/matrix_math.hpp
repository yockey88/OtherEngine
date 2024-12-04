/**
 * \file math/matrix_math.hpp
 **/
#ifndef OTHER_ENGINE_MATRIX_MATH_HPP
#define OTHER_ENGINE_MATRIX_MATH_HPP

#include <glm/glm.hpp>

namespace other {

  void DecomposeTransformMatrix(const glm::mat4& matrix, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale);

}  // namespace other

#endif  // !OTHER_ENGINE_MATRIX_MATH_HPP
