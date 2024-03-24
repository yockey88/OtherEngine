/**
 * \file rendering/ui/ui_colors.cpp
 */
#include "rendering/ui/ui_colors.hpp"

#include <algorithm>

namespace other {
namespace ui {
namespace theme {

  ImU32 ColorWithMultiplier(const ImColor& color , float multiplier){
    const ImVec4& color_val = color.Value;
    float hue , sat , val;
    ImGui::ColorConvertRGBtoHSV(color_val.x , color_val.y , color_val.z , hue , sat , val);
    return ImColor::HSV(hue , sat , std::min(val * multiplier , 1.f));
  }

} // namespace theme
} // namespace ui
} // namespace other
