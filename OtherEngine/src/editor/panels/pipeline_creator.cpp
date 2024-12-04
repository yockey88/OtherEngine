/**
 * \file editor/panels/pipeline_creator.cpp
 **/
#include "editor/panels/pipeline_creator.hpp"

#include <imgui/imgui.h>

#include "application/app_state.hpp"

#include "scene/scene_manager.hpp"

#include "rendering/ui/ui_helpers.hpp"

namespace other {

  void PipelineCreator::OnAttach() {
    pipeline_name.fill('\0');
  }

  void PipelineCreator::OnDetach() {
  }

  /// TODO: add customizeable vertex layout and uniforms
  bool PipelineCreator::OnGuiRender(bool& is_open) {
    if (!ImGui::Begin("Pipeline Creator", &is_open)) {
      ImGui::End();
      return false;
    }

    ui::Button("Finalize Pipeline", [&]() {
      spec.vertex_layout = {
        { ValueType::VEC3, "position" },
        { ValueType::VEC3, "normal" },
        { ValueType::VEC3, "tangent" },
        { ValueType::VEC3, "binormal" },
        { ValueType::VEC2, "uvs" }
      };

      spec.model_binding_point = 1;
      spec.model_uniforms = {
        { "models", ValueType::MAT4, 100 },
      };

      spec.material_binding_point = 2;
      spec.material_uniforms = {
        { "materials", ValueType::USER_TYPE, 100, sizeof(Material) },
      };

      spec.pipeline_name = std::string{ pipeline_name.data() };

      Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
      OE_ASSERT(renderer != nullptr, "No scene renderer found");

      renderer->AddPipeline(spec);

      is_open = false;
    });

    ImGui::Text("Pipeline Specification");

    if (ImGui::InputText("Pipeline Name", pipeline_name.data(), pipeline_name.size())) {}

    ImGui::Separator();
    if (ui::PropertyDropdown("Draw Mode", draw_modes, draw_mode_count, selected_draw_mode)) {
      spec.topology = static_cast<DrawMode>(selected_draw_mode);
    }
    ImGui::Separator();
    if (ui::Checkbox("Back Face Culling", &spec.back_face_culling)) {}
    ImGui::Separator();
    if (ui::Checkbox("Depth Test", &spec.depth_test)) {}
    ImGui::Separator();
    if (ui::DragFloat("Line Width", &spec.line_width, 0.1f, 0.f, 10.f)) {}
    ImGui::Separator();
    ImGui::Separator();

    ImGui::Text("Framebuffer Specification");
    ImGui::Separator();
    if (ui::PropertyDropdown("Depth Functions", depth_funcs, depth_func_count, selected_depth_func)) {
      spec.framebuffer_spec.depth_func = static_cast<DepthFunction>(selected_depth_func);
    }
    ImGui::Separator();
    if (ImGui::ColorEdit4("Clear Color", glm::value_ptr(spec.framebuffer_spec.clear_color))) {}
    ImGui::Separator();
    if (ImGui::DragInt2("Size", glm::value_ptr(spec.framebuffer_spec.size), 1, 0, 4096)) {}
    ImGui::Separator();
    if (ui::Checkbox("Depth Buffer", &spec.framebuffer_spec.depth)) {}
    ImGui::Separator();
    if (ui::Checkbox("Color Buffer", &spec.framebuffer_spec.color)) {}
    ImGui::Separator();
    if (ui::Checkbox("Stencil Buffer", &spec.framebuffer_spec.stencil)) {}
    ImGui::Separator();
    ImGui::Separator();

    ImGui::End();
    return false;
  }

}  // namespace other
