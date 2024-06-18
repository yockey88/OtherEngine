/**
 * \file scripting/cs/native_functions/cs_native_rendering.hpp
 **/
#ifndef OTHER_ENGINE_CS_NATIVE_RENDERING_HPP
#define OTHER_ENGINE_CS_NATIVE_RENDERING_HPP

#include "rendering/color.hpp"
#include "rendering/primitives.hpp"

namespace other {
namespace cs_script_bindings {

  void NativeDrawLine(const Line* line , const RgbColor* color);
  void NativeDrawTriangle(const Triangle* triangle , const RgbColor* color);
  void NativeDrawRect(const Rect* rect , const RgbColor* color);

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_RENDERING_HPP
