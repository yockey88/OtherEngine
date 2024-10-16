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

  bool DrawVec2Control(const std::string_view label , glm::vec2& , bool& edited , 
                       float reset_val = 0.f , float col_w = 100.f , VectorAxis axes = VectorAxis::ZERO , 
                       const glm::vec2& v_min = glm::zero<glm::vec2>() , const glm::vec2& v_max = glm::zero<glm::vec2>() ,
                       float speed = 1.f);
  
  bool EditVec2(const std::string_view label , ImVec2 size , float reset_val , bool& edited , glm::vec2& value ,
                VectorAxis axes , float speed = 1.f , const glm::vec2& v_min = glm::zero<glm::vec2>() , 
                const glm::vec2& v_max = glm::zero<glm::vec2>() , const char* format = "%.2f" , ImGuiSliderFlags flags = 0);

  bool DrawVec3Control(const std::string_view label , glm::vec3& value , bool& edited , 
                       float reset_val = 0.f , float col_w = 100.f , VectorAxis axes = VectorAxis::ZERO , 
                       const glm::vec3& v_min = glm::zero<glm::vec3>() , const glm::vec3& v_max = glm::zero<glm::vec3>() ,
                       float speed = 1.f);
  
  bool EditVec3(const std::string_view label , ImVec2 size , float reset_val , bool& edited , glm::vec3& value ,
                VectorAxis axes , float speed = 1.f , const glm::vec3& v_min = glm::zero<glm::vec3>() , 
                const glm::vec3& v_max = glm::zero<glm::vec3>() , const char* format = "%.2f" , ImGuiSliderFlags flags = 0);
  
  bool DrawVec4Control(const std::string_view label , glm::vec4& , bool& edited , 
                       float reset_val = 0.f , float col_w = 100.f , VectorAxis axes = VectorAxis::ZERO , 
                       const glm::vec4& v_min = glm::zero<glm::vec4>() , const glm::vec4& v_max = glm::zero<glm::vec4>() ,
                       float speed = 1.f);
  
  bool EditVec4(const std::string_view label , ImVec2 size , float reset_val , bool& edited , glm::vec4& value ,
                VectorAxis axes , float speed = 1.f , const glm::vec4& v_min = glm::zero<glm::vec4>() , 
                const glm::vec4& v_max = glm::zero<glm::vec4>() , const char* format = "%.2f" , ImGuiSliderFlags flags = 0);

} // namespace widgets 
} // namespace ui
} // namespace other

#endif // !OTHER_ENGINE_UI_WIDGETS_HPP
