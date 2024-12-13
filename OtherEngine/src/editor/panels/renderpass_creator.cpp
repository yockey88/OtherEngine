/**
 * \file editor/panels/renderpass_creator.cpp
 **/
#include "editor/panels/renderpass_creator.hpp"

#include <imgui/imgui.h>

#include "core/filesystem.hpp"

#include "application/app_state.hpp"

#include "rendering/ui/ui_helpers.hpp"

namespace other {

  void RenderpassCreator::OnAttach() {
    Ref<Directory> project_dir = Filesystem::GetDirectory("project-root");
    pass_name.fill('\0');

    auto other_files = project_dir->GetFiles(".oshader");
    for (auto& file : other_files) {
      OE_ASSERT(file != nullptr, "Failed to load other shader file");
      other_shader_files[file->handle.Get()] = file;
    }
  }

  void RenderpassCreator::OnDetach() {
    other_shader_files.clear();
  }

  bool RenderpassCreator::OnGuiRender(bool& is_open) {
    if (!ImGui::Begin("Renderpass Creator", &is_open)) {
      ImGui::End();
      return false;
    }

    ui::Button("Finalize Renderpass", [&]() {
      spec.name = pass_name.data();
      if (spec.name.empty()) {
        OE_ERROR("Renderpass name is empty!");
        return;
      }

      if (spec.shader == nullptr) {
        OE_ERROR("No shader selected for renderpass");
        return;
      }

      // AppState::Scenes()->GetRenderer()->AddRenderPass(spec);
    });

    ImGui::Text("Renderpass Specification");

    // App& app = AppState::AppHandle();
    // for (auto& fb : app.render_specs.framebuffer_specs) {
    //   if (fb.framebuffer_name.empty()) {
    //     ImGui::Text("Nameless Framebuffer");
    //   } else {
    //     ImGui::Text("Framebuffer: %s", fb.framebuffer_name.c_str());
    //   }

    //   ImGui::ColorEdit4("clear color", glm::value_ptr(fb.clear_color));
    //   ImGui::DragInt2("size", glm::value_ptr(fb.size));

    //   ui::Checkbox("clear depth buffer", &fb.depth);
    //   ui::Checkbox("clear stencil buffer", &fb.stencil);
    //   ui::Checkbox("clear color buffer", &fb.color);
    // }
    // for (auto& pass : render_data.render_passes) {
    //   ImGui::Text("Renderpass: %s", pass.name.c_str());
    // }
    // for (auto& pipe : render_data.pipelines) {
    //   ImGui::Text("Pipeline: %s", pipe.pipeline_name.c_str());
    // }

    ImGui::Separator();

    if (ImGui::InputText("Renderpass Name", pass_name.data(), pass_name.size())) {}
    ImGui::Separator();

    if (spec.shader == nullptr) {
      ImGui::Text("Shaders");
      for (auto& [id, file] : other_shader_files) {
        ImGui::PushID(id.Get());
        if (ui::Selectable(file->FileName().c_str())) {
          Ref<Shader> shader = BuildShader(file->AbsolutePath());
          if (shader != nullptr) {
            spec.shader = shader;
            ImGui::PopID();
            break;
          } else {
            OE_ERROR("Failed to build shader from file: {}", file->FileName());
          }
        }
        ImGui::PopID();
      }
    }

    ImGui::Separator();
    if (spec.shader != nullptr) {
      {
        ScopedColor green_text(ImGuiCol_Text, ui::theme::green);
        ImGui::Text("Current Shader: %s", spec.shader->Name().c_str());
      }
      const auto& uniforms = spec.shader->GetIr().uniforms;
      ImGui::Text("Uniforms");
      ImGui::Separator();
      for (auto& [id, uni] : uniforms) {
        std::string type_str = fmtstr("{}", uni.type);
        ImGui::Text("Uniform: %s [%s]", uni.name.c_str(), type_str.c_str());
      }

      ImGui::Text("Inputs");
      ImGui::Separator();
      for (auto& [id, in] : spec.shader->GetIr().inouts) {
        if (in.in_out == InOutType::INPUT) {
          std::string type_str = fmtstr("{}", in.type);
          ImGui::Text("Input: %s [%s]", in.name.c_str(), type_str.c_str());
        }
      }

      ImGui::Text("Outputs");
      ImGui::Separator();
      for (auto& [id, out] : spec.shader->GetIr().inouts) {
        if (out.in_out == InOutType::OUTPUT) {
          std::string type_str = fmtstr("{}", out.type);
          ImGui::Text("Output: %s [%s]", out.name.c_str(), type_str.c_str());
        }
      }
    }

    /// uniform editor
    /// shader maker

    ImGui::End();
    return false;
  }

}  // namespace other
