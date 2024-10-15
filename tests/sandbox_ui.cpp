/**
 * \file sandbox/sandbox_ui.cpp
 **/
#include "sandbox_ui.hpp"

#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"

void RenderItem(uint32_t id, const std::string& title, ImVec2 size) {
  ImGui::PushID(("##" + title).c_str());
  ImGui::Text("%s", title.c_str());
  ImGui::Image((ImTextureID)(uintptr_t)id, ImVec2(size.x, size.y), ImVec2(0, 1), ImVec2(1, 0));
  ImGui::PopID();
}

bool RenderMaterial(const std::string& title, other::Material& mat) {
  ImGui::PushID(("##" + title).c_str());
  ImGui::Text("%s", title.c_str());

  bool edited = false;

  glm::vec3 temp_color = mat.color;
  edited = (other::ui::widgets::DrawVec3Control("color", temp_color, edited, 0.f, 100.f, other::ui::VectorAxis::ZERO,
                                                { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f }, 0.1f) &&
            edited);
  other::ui::BeginProperty("shininess");
  edited = other::ui::DragFloat("##shininess", &mat.shininess, 0.5f, 0.f, 256.f) && edited;
  other::ui::EndProperty();

  mat.color = glm::vec4(temp_color, 1.f);

  ImGui::PopID();

  return edited;
}

bool RenderPointLight(const std::string& title, other::PointLight& pl) {
  ImGui::PushID(("##" + title).c_str());
  ImGui::Text("%s", title.c_str());

  bool edited = false;

  glm::vec3 temp_position = pl.position;
  glm::vec3 temp_color = pl.color;
  edited = edited || other::ui::widgets::DrawVec3Control("position", temp_position, edited, 0.f, 100.f, other::ui::VectorAxis::ZERO, { -100.f, -100.f, -100.f }, { 100.f, 100.f, 100.f }, 0.5f);
  edited = edited || other::ui::widgets::DrawVec3Control("ambient color", temp_color, edited, 0.f, 100.f, other::ui::VectorAxis::ZERO, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f }, 0.1f);
  other::ui::BeginProperty("radius");
  edited = edited || other::ui::DragFloat("##radius", &pl.radius, 0.1f, 0.f, 256.f);
  other::ui::EndProperty();

  other::ui::BeginProperty("constant");
  edited = edited || other::ui::DragFloat("##constant", &pl.constant, 0.1f, 0.f, 256.f);
  other::ui::EndProperty();

  other::ui::BeginProperty("linear");
  edited = edited || other::ui::DragFloat("##linear", &pl.linear, 0.05f, 0.f, 256.f);
  other::ui::EndProperty();

  other::ui::BeginProperty("quadratic");
  edited = edited || other::ui::DragFloat("##quadratic", &pl.quadratic, 0.01f, 0.f, 256.f);
  other::ui::EndProperty();

  pl.position = glm::vec4(temp_position, 1.f);
  pl.color = glm::vec4(temp_color, 1.f);
  ImGui::PopID();

  return edited;
}

bool RenderDirectionLight(const std::string& title, other::DirectionLight& dl) {
  ImGui::PushID(("##" + title).c_str());
  ImGui::Text("%s", title.c_str());

  bool edited = false;

  glm::vec3 temp_direction = dl.direction;
  glm::vec3 temp_color = dl.color;
  edited = edited || other::ui::widgets::DrawVec3Control("direction", temp_direction, edited, 0.f, 100.f, other::ui::VectorAxis::ZERO, { -100.f, -100.f, -100.f }, { 100.f, 100.f, 100.f }, 0.5f);
  edited = edited || other::ui::widgets::DrawVec3Control("color", temp_color, edited, 0.f, 100.f, other::ui::VectorAxis::ZERO, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f }, 0.1f);

  dl.direction = glm::vec4(temp_direction, 1.f);
  dl.color = glm::vec4(temp_color, 1.f);
  ImGui::PopID();

  return edited;
}

void RenderSettings() {
}
