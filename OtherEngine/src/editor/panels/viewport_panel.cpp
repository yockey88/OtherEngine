/**
 * \file editor/panels/viewport_panel.cpp
 **/
#include "editor/panels/viewport_panel.hpp"

#include <glm/fwd.hpp>
#include <imgui/imgui.h>

#include "editor/editor_settings.hpp"
#include "editor/editor_state.hpp"

#include "application/app_state.hpp"

#include "rendering/renderer.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {

  void ViewportPanel::OnRender() {
  }

  bool ViewportPanel::OnGuiRender(bool& is_open) {
    if (!is_open) {
      return false;
    }

    if (AppState::Scenes()->HasActiveScene()) {
      EditorSettings& settings = EditorSettings::Get();
      EditorState& editor = EditorState::Get();

      Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
      OE_ASSERT(renderer != nullptr, "No renderer found in scene");

      /// FIXME: frame is hardcodede
      auto& frames = renderer->GetRender();
      Ref<Framebuffer> viewport = nullptr;
      auto itr = frames.find(FNV(settings.viewport_frame_name));
      if (itr == frames.end()) {
        // default to geometry frame
        itr = frames.find(FNV("Geometry"));
      }
      viewport = itr->second;

      if (ImGui::Begin("Viewport")) {
        ImVec2 cursor_pos = ImGui::GetCursorPos();
        if (viewport == nullptr) {
          ScopedColor red_text(ImGuiCol_Text, ui::theme::red);
          ImGui::Text("No Viewport Generated");
          editor.viewport_mouse_pos = std::nullopt;
        } else {
          ImVec2 curr_win_size = ImGui::GetContentRegionAvail();

          float aspect_ratio = Renderer::GetWindow()->AspectRatio();
          float ui_aspect_ratio = curr_win_size.x / curr_win_size.y;

          glm::vec2 padding = { 0.f, 0.f };
          glm::vec2 size = { curr_win_size.x, curr_win_size.y };
          if (ui_aspect_ratio > aspect_ratio) {
            size.x = curr_win_size.y * aspect_ratio;
            padding = { (curr_win_size.x - size.x) * 0.5f, 0.f };
          } else {
            size.y = curr_win_size.x / aspect_ratio;
            padding = { 0.f, (curr_win_size.y - size.y) * 0.5f };
          }

          ImTextureID tex_id = (void*)(uintptr_t)viewport->texture;
          ImVec2 img_size = { size.x, size.y };

          ImGui::SetCursorPos({ padding.x, padding.y });
          ImGui::Image(tex_id, img_size, ImVec2(0, 1), ImVec2(1, 0));
          ImGui::SetCursorPos(cursor_pos);

          if (!ImGui::IsWindowHovered()) {
            editor.viewport_mouse_pos = std::nullopt;
          } else {
            glm::vec2 win_pos = {
              ImGui::GetWindowPos().x,
              ImGui::GetWindowPos().y + (padding.y / 2.f)
            };
            ImVec2 imgui_mouse_pos = ImGui::GetMousePos();

            editor.viewport_bounds[0] = {
              win_pos.x + cursor_pos.x,
              win_pos.y + cursor_pos.y
            };
            editor.viewport_bounds[1] = {
              editor.viewport_bounds[0].x + img_size.x,
              editor.viewport_bounds[0].y + img_size.y
            };

            editor.viewport_mouse_pos = {
              imgui_mouse_pos.x - editor.viewport_bounds[0].x,
              imgui_mouse_pos.y - editor.viewport_bounds[0].y
            };
            editor.current_viewport_size = vec2_sub(editor.viewport_bounds[1], editor.viewport_bounds[0]);
            editor.viewport_mouse_pos->y = editor.current_viewport_size.y - editor.viewport_mouse_pos->y;
          }
        }
      }
      ImGui::End();
    }
    return is_open;
  }

  void ViewportPanel::OnProjectChange(const Ref<Project>& project) {
  }

  void ViewportPanel::SetSceneContext(const Ref<Scene>& scene) {
    active_scene = scene;
  }

}  // namespace other
