/**
 * \file rendering/ui/ui_colors.cpp
 */
#include "rendering/ui/ui_colors.hpp"

#include <algorithm>
#include <imgui/imgui.h>

namespace other {
namespace ui {
namespace theme {

  ImU32 ColorWithMultiplier(const ImColor& color , float multiplier){
    const ImVec4& color_val = color.Value;
    float hue , sat , val;
    ImGui::ColorConvertRGBtoHSV(color_val.x , color_val.y , color_val.z , hue , sat , val);
    return ImColor::HSV(hue , sat , std::min(val * multiplier , 1.f));
  }
  
  ImU32 ColorWithMultipliedSaturation(const ImColor& color , float multiplier) {
    const ImVec4& col_raw = color.Value;
    float hue , sat , val;
    ImGui::ColorConvertRGBtoHSV(col_raw.x , col_raw.y , col_raw.z , hue , sat , val);
    return ImColor::HSV(hue , std::min(sat * multiplier , 1.0f) , val);
  }

} // namespace theme
} // namespace ui
} // namespace other
