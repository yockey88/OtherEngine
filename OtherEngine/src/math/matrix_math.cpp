/**
 * \file math/matrix_math.cpp
 **/
#include "math/matrix_math.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/quaternion.hpp>

#include "core/logger.hpp"
#include "math/vecmath.hpp"

namespace other {

  bool DecomposeTransformMatrix(const glm::mat4& matrix, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale) {
    /* Taken from glm/gtx/matrix_decompose.inl */
    using namespace glm;
    using T = float;

    mat4 local_matrix(matrix);

    /// normalize the matrix
    for (length_t i = 0; i < 3; ++i) {
      local_matrix[i] = vec4_div(local_matrix[i], local_matrix[3][3]);
    }

    OE_ASSERT(epsilonEqual(local_matrix[3][3], static_cast<T>(1), epsilon<T>()), "Matrix is not normalized");
    OE_ASSERT(
      epsilonEqual(local_matrix[0][3], static_cast<T>(0), epsilon<T>()) &&
        epsilonEqual(local_matrix[1][3], static_cast<T>(0), epsilon<T>()) &&
        epsilonEqual(local_matrix[2][3], static_cast<T>(0), epsilon<T>()),
      "Matrix is not normalized"
    );

    // Next take care of translation (easy).
    translation = vec3(local_matrix[3]);
    local_matrix[3] = vec4(0, 0, 0, local_matrix[3].w);

    vec3 row[3];

    // Now get scale and shear.
    for (length_t i = 0; i < 3; ++i) {
      for (length_t j = 0; j < 3; ++j) {
        row[i][j] = local_matrix[i][j];
      }
    }

    // Compute X scale factor and normalize first row.
    scale.x = length(row[0]);
    row[0] = scale_vec3(row[0], static_cast<T>(1));
    scale.y = length(row[1]);
    row[1] = scale_vec3(row[1], static_cast<T>(1));
    scale.z = length(row[2]);
    row[2] = scale_vec3(row[2], static_cast<T>(1));

    // At this point, the matrix (in rows[]) is orthonormal.
    // Check for a coordinate system flip.  If the determinant
    // is -1, then negate the matrix and the scaling factors.
    vec3 pdum = cross(row[1], row[2]);
    if (dot(row[0], pdum) < 0) {
      for (length_t i = 0; i < 3; i++) {
        scale[i] *= static_cast<T>(-1);
        row[i] *= static_cast<T>(-1);
      }
    }

    int32_t i, j, k;
    T root;
    T trace = row[0].x + row[1].y + row[2].z;
    if (trace > static_cast<T>(0)) {
      root = sqrt(trace + static_cast<T>(1));
      rotation.w = static_cast<T>(0.5) * root;
      root = static_cast<T>(0.5) / root;
      rotation.x = root * (row[1].z - row[2].y);
      rotation.y = root * (row[2].x - row[0].z);
      rotation.z = root * (row[0].y - row[1].x);
    } else {
      static int32_t next[3] = { 1, 2, 0 };
      i = 0;
      if (row[1].y > row[0].x) {
        i = 1;
      }
      if (row[2].z > row[i][i]) {
        i = 2;
      }

      j = next[i];
      k = next[j];

      root = sqrt(row[i][i] - row[j][j] - row[k][k] + static_cast<T>(1));

      rotation[i] = static_cast<T>(0.5) * root;
      root = static_cast<T>(0.5) / root;
      rotation[j] = root * (row[i][j] + row[j][i]);
      rotation[k] = root * (row[i][k] + row[k][i]);
      rotation.w = root * (row[j][k] - row[k][j]);
    }

    return true;
  }

}  // namespace other
