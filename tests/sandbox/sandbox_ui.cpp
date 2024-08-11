/**
 * \file sandbox/sandbox_ui.cpp
 **/
#include "sandbox_ui.hpp"

#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"

void RenderItem(int32_t id , const std::string& title , ImVec2 size) {
  ImGui::PushID(("##" + title).c_str());
  ImGui::Text("%s" , title.c_str());
  ImGui::SameLine();
  ImGui::Image((void*)(intptr_t)id , ImVec2(size.x , size.y) , ImVec2(0 , 1) , ImVec2(1 , 0));
  ImGui::PopID();
}

void RenderMaterial(const std::string& title , other::Material& mat) {
  ImGui::PushID(("##" + title).c_str());
  ImGui::Text("%s" , title.c_str());

  bool edited = false;

  glm::vec3 temp_ambient = mat.ambient;
  // glm::vec3 temp_diffuse = mat.diffuse;
  // glm::vec3 temp_specular = mat.specular;
  other::ui::widgets::DrawVec3Control("ambient color" , temp_ambient , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                      { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
  // other::ui::widgets::DrawVec3Control("diffuse color" , temp_diffuse , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
  //                                     { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
  // other::ui::widgets::DrawVec3Control("specular color" , temp_specular , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
  //                                     { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
  // other::ui::BeginProperty("shininess");
  // other::ui::DragFloat("##shininess" , &mat.shininess , 0.5f , 0.f , 256.f);
  // other::ui::EndProperty();

  mat.ambient = glm::vec4(temp_ambient , 1.f);
  // mat.diffuse = glm::vec4(temp_diffuse , 1.f);
  // mat.specular = glm::vec4(temp_specular , 1.f);
  
  ImGui::PopID();
}

void RenderPointLight(const std::string& title , other::PointLight& pl) {
  ImGui::PushID(("##" + title).c_str());
  ImGui::Text("%s" , title.c_str());

  bool edited = false;
  
  glm::vec3 temp_position = pl.position;
  glm::vec3 temp_ambient = pl.ambient;
  glm::vec3 temp_diffuse = pl.diffuse;
  glm::vec3 temp_specular = pl.specular;
  other::ui::widgets::DrawVec3Control("position" , temp_position , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                      { -100.f , -100.f , -100.f } , { 100.f , 100.f , 100.f } , 0.5f);
  other::ui::widgets::DrawVec3Control("ambient color" , temp_ambient , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                      { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
  other::ui::widgets::DrawVec3Control("diffuse color" , temp_diffuse , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                      { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
  other::ui::widgets::DrawVec3Control("specular color" , temp_specular , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                      { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
  other::ui::BeginProperty("constant");
  other::ui::DragFloat("##constant" , &pl.constant , 0.1f , 0.f , 256.f);
  other::ui::EndProperty();

  other::ui::BeginProperty("linear");
  other::ui::DragFloat("##linear" , &pl.linear , 0.05f , 0.f , 256.f);
  other::ui::EndProperty();

  other::ui::BeginProperty("quadratic");
  other::ui::DragFloat("##quadratic" , &pl.quadratic , 0.01f , 0.f , 256.f);
  other::ui::EndProperty();

  pl.position = glm::vec4(temp_position , 1.f);
  pl.ambient = glm::vec4(temp_ambient , 1.f);
  pl.diffuse = glm::vec4(temp_diffuse , 1.f);
  pl.specular = glm::vec4(temp_specular , 1.f);
  
  ImGui::PopID();
}

void RenderDirectionLight(const std::string& title , other::DirectionLight& dl) {
  ImGui::PushID(("##" + title).c_str());
  ImGui::Text("%s" , title.c_str());

  bool edited = false;

  glm::vec3 temp_direction = dl.direction;
  glm::vec3 temp_ambient = dl.ambient;
  glm::vec3 temp_diffuse = dl.diffuse;
  glm::vec3 temp_specular = dl.specular;
  other::ui::widgets::DrawVec3Control("direction" , temp_direction , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                      { -100.f , -100.f , -100.f } , { 100.f , 100.f , 100.f } , 0.5f);
  other::ui::widgets::DrawVec3Control("ambient color" , temp_ambient , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                      { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
  other::ui::widgets::DrawVec3Control("diffuse color" , temp_diffuse , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                      { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
  other::ui::widgets::DrawVec3Control("specular color" , temp_specular , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                      { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
  
  dl.direction = glm::vec4(temp_direction , 1.f);
  dl.ambient = glm::vec4(temp_ambient , 1.f);
  dl.diffuse = glm::vec4(temp_diffuse , 1.f);
  dl.specular = glm::vec4(temp_specular , 1.f);
  ImGui::PopID();
}

void RenderSettings() {
}
