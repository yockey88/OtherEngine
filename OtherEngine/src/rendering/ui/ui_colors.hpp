/**
 * \file rendering/ui/colors.hpp
 **/
#ifndef OTHER_ENGINE_UI_COLORS_HPP
#define OTHER_ENGINE_UI_COLORS_HPP

#include <imgui/imgui.h>

namespace other {
namespace ui {
namespace theme {

  constexpr auto blue = IM_COL32(0 , 0 , 255 , 255);
  constexpr auto accent = IM_COL32(236 , 158 , 36 , 255);
  constexpr auto highlight = IM_COL32(39 , 185 , 242 , 255);
  /* ?? */ constexpr auto nice_blue = IM_COL32(83 , 232 , 254 , 255);
  constexpr auto compliment = IM_COL32(78 , 151 , 166 , 255);
  constexpr auto background = IM_COL32(36 , 36 , 36 , 255);
  constexpr auto background_dark = IM_COL32(26 , 26 , 26 , 255);
  constexpr auto title_bar = IM_COL32(21 , 21 , 21 , 255);
  constexpr auto title_bar_orange = IM_COL32(186 , 66 , 30 , 255);
  constexpr auto title_bar_green = IM_COL32(18 , 88 , 30 , 255);
  constexpr auto title_bar_red = IM_COL32(185 , 30 , 30 , 255);
  constexpr auto property_field = IM_COL32(15 , 15 , 15 , 255);
  constexpr auto text = IM_COL32(192 , 192 , 192 , 255);
  constexpr auto text_bright = IM_COL32(210 , 210 , 210 , 255);
  constexpr auto text_dark = IM_COL32(128 , 128 , 128 , 255);
  constexpr auto text_error = IM_COL32(230, 51, 51, 255);
  constexpr auto muted = IM_COL32(77, 77, 77, 255);
  constexpr auto group_header = IM_COL32(47, 47, 47, 255);
  constexpr auto selection = IM_COL32(237, 192, 119, 255);
  constexpr auto selection_muted  = IM_COL32(237, 201, 142, 23);
  constexpr auto background_popup = IM_COL32(50, 50, 50, 255);
  constexpr auto valid_prefab = IM_COL32(82, 179, 222, 255);
  constexpr auto invalid_prefab = IM_COL32(222, 43, 43, 255);
  constexpr auto missing_mesh = IM_COL32(230, 102, 76, 255);
  constexpr auto mesh_not_set = IM_COL32(250, 101, 23, 255);

  ImU32 ColorWithMultiplier(const ImColor& color , float multiplier);

} // namespace theme
} // namespace ui
} // namespace other

#endif // !OTHER_ENGINE_UI_COLORS_HPP
