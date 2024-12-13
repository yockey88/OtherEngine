/**
 * \file math/matrix_math.cpp
 **/
#include "math/matrix_math.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/quaternion.hpp>

#include "math/vecmath.hpp"

namespace other {

  bool DecomposeTransformMatrix(const glm::mat4& matrix, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale) {
    /* Taken from glm/gtx/matrix_decompose.inl */
    using namespace glm;
    using T = float;

    mat4 local_matrix(matrix);

    // Normalize the matrix.
    if (epsilonEqual(local_matrix[3][3], static_cast<T>(0), epsilon<T>()))
      return false;

    if (epsilonNotEqual(local_matrix[0][3], static_cast<T>(0), epsilon<T>()) ||
        epsilonNotEqual(local_matrix[1][3], static_cast<T>(0), epsilon<T>()) ||
        epsilonNotEqual(local_matrix[2][3], static_cast<T>(0), epsilon<T>())) {
      // Clear the perspective partition
      local_matrix[0][3] = local_matrix[1][3] = local_matrix[2][3] = static_cast<T>(0);
      local_matrix[3][3] = static_cast<T>(1);
    }

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

    /*
        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
        Pdum3 = cross(row[1], row[2]); // v3Cross(row[1], row[2], Pdum3);
        if(dot(row[0], Pdum3) < 0)
        {
            for(length_t i = 0; i < 3; i++)
            {
                Scale[i] *= static_cast<T>(-1);
                row[i] *= static_cast<T>(-1);
            }
        }
    */

    // Now, get the rotations out, as described in the gem.

    // FIXME - Add the ability to return either quaternions (which are
    // easier to recompose with) or Euler angles (rx, ry, rz), which
    // are easier for authors to deal with. The latter will only be useful
    // when we fix https://bugs.webkit.org/show_bug.cgi?id=23799, so I
    // will leave the Euler angle code here for now.
    glm::vec3 rot = { 0.f, 0.f, 0.f };
    rot.y = asin(-row[0][2]);
    if (cos(rot.y) != 0) {
      rot.x = atan2(row[1][2], row[2][2]);
      rot.z = atan2(row[0][1], row[0][0]);
    } else {
      rot.x = atan2(-row[2][0], row[1][1]);
      rot.z = 0;
    }

    rotation = glm::quat(rot);
    return true;
  }

}  // namespace other
