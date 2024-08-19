/**
 * \file gl_helpers.hpp
 **/
#ifndef GL_HELPERS_HPP
#define GL_HELPERS_HPP

#include <cstdint>

#define CHECK() do { other::CheckGlError(__LINE__); } while (false)

namespace other {

  void CheckGlError(int line);

  uint32_t GetShader(const char* vsrc , const char* fsrc);

} // namespace other

#endif // !GL_HELPERS_HPP
