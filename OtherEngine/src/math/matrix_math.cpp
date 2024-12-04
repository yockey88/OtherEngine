/**
 * \file math/matrix_math.cpp
 **/
#include "math/matrix_math.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/quaternion.hpp>

#include "math/vecmath.hpp"

namespace other {

  void DecomposeTransformMatrix(const glm::mat4& matrix, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale) {
    glm::mat4 local_matrix = matrix;
    if (glm::epsilonEqual(matrix[3][3], 0.f, glm::epsilon<float>())) {
      return;
    }

    translation = glm::vec3(local_matrix[3]);
    local_matrix[3] = { 0.f, 0.f, 0.f, local_matrix[3].w };

    glm::vec3 row[3];
    for (glm::length_t i = 0; i < 3; ++i) {
      for (glm::length_t j = 0; j < 3; ++j) {
        row[i][j] = local_matrix[i][j];
      }
    }

    scale.x = glm::length(row[0]);
    scale.y = glm::length(row[1]);
    scale.z = glm::length(row[2]);

    row[0] = scale_vec3(row[0], 1.f);
    row[1] = scale_vec3(row[1], 1.f);
    row[2] = scale_vec3(row[2], 1.f);

    // Rotation as quaternion
    int32_t i = 0;
    int32_t j = 0;
    int32_t k = 0;
    float root, trace = row[0].x + row[1].y + row[2].z;
    if (EpsilonGt(trace, 0.f)) {
      root = glm::sqrt(trace + 1.f);
      rotation.w = 0.5f * root;

      root = 0.5f / root;
      rotation.x = root * (row[1].z - row[2].y);
      rotation.y = root * (row[2].x - row[0].z);
      rotation.z = root * (row[0].y - row[1].x);
    } else {
      static int32_t Next[3] = { 1, 2, 0 };
      i = 0;
      if (row[1].y > row[0].x) {
        i = 1;
      }
      if (row[2].z > row[i][i]) {
        i = 2;
      }
      j = Next[i];
      k = Next[j];

      root = glm::sqrt(row[i][i] - row[j][j] - row[k][k] + 1.f);

      rotation[i] = 0.5f * root;
      root = 0.5f / root;
      rotation[j] = root * (row[i][j] + row[j][i]);
      rotation[k] = root * (row[i][k] + row[k][i]);
      rotation.w = root * (row[j][k] - row[k][j]);
    }
  }

}  // namespace other
