/**
 * \file scripting/cs/native_functions/cs_native_rendering.cpp
 **/
#include "scripting/cs/native_functions/cs_native_rendering.hpp"

#include "rendering/renderer.hpp"

namespace other {
namespace cs_script_bindings {

  void NativeDrawLine(const Line* line , const RgbColor* color) {
    Renderer::DrawLine(*line , *color);
  }

  void NativeDrawTriangle(const Triangle* triangle , const RgbColor* color) {
    Renderer::DrawTriangle(*triangle , *color);
  }
  
  void NativeDrawRect(const Rect* rect , const RgbColor* color) {
    Renderer::DrawRect(*rect , *color);
  }

} // cs_script_bindings
} // namespace other
