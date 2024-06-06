/**
 * \file rendering/ui/ui_widgets.hpp
 **/
#ifndef OTHER_ENGINE_UI_WIDGETS_HPP
#define OTHER_ENGINE_UI_WIDGETS_HPP

#include "core/defines.hpp"

#include <imgui/imgui.h>
#include <glm/gtc/constants.hpp>

namespace other {
namespace ui {

  enum VectorAxis {
    ZERO = 0 ,
    X = bit(1) ,
    Y = bit(2) ,
    Z = bit(3) ,
    W = bit(4)
  };

namespace widgets {

  bool DrawVec3Control(const std::string_view label , glm::vec3& value , bool& edited , 
                       float reset_val = 0.f , float col_w = 100.f , VectorAxis axes = VectorAxis::ZERO);
  
  bool EditVec3(const std::string_view label , ImVec2 size , float reset_val , bool& edited , glm::vec3& value ,
                VectorAxis axes , float speed = 1.f , const glm::vec3& v_min = glm::zero<glm::vec3>() , 
                const glm::vec3& v_max = glm::zero<glm::vec3>() , const char* format = "%.2f" , ImGuiSliderFlags flags = 0);

} // namespace widgets 
} // namespace ui
} // namespace other

#endif // !OTHER_ENGINE_UI_WIDGETS_HPP
