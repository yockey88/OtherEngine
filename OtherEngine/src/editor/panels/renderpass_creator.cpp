/**
 * \file editor/panels/renderpass_creator.cpp
 **/
#include "editor/panels/renderpass_creator.hpp"

#include <imgui/imgui.h>

#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"

#include "rendering/ui/ui_helpers.hpp"

namespace other {

  void RenderpassCreator::OnAttach() {
    pass_name.fill('\0');
  }

  bool RenderpassCreator::OnGuiRender(bool& is_open) {
    if (!ImGui::Begin("Renderpass Creator", &is_open)) {
      ImGui::End();
      return false;
    }

    ui::Button("Finalize Renderpass", [&]() {
    });

    ImGui::Text("Renderpass Specification");

    if (ImGui::InputText("Renderpass Name", pass_name.data(), pass_name.size())) {}
    ImGui::Separator();

    std::set<AssetHandle> shaders = AppState::Assets()->GetAllOfType(AssetType::SHADER);
    ImGui::Text("Shaders");
    ImGui::Separator();

    for (auto& shader : shaders) {
      Ref<Shader> s = AssetManager::GetAsset<Shader>(shader);
      OE_ASSERT(s != nullptr, "Failed to get shader from asset manager");

      if (ImGui::Button(s->Name().c_str())) {
        if (spec.shader == s) {
          spec.shader = nullptr;
        } else {
          spec.shader = s;
        }
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
