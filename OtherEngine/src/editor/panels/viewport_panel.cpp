/**
 * \file editor/panels/viewport_panel.cpp
 **/
#include "editor/panels/viewport_panel.hpp"

#include <glm/fwd.hpp>
#include <imgui/imgui.h>

#include "application/app_state.hpp"

#include "rendering/renderer.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "editor/editor_settings.hpp"
#include "editor/editor_state.hpp"

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
        ImVec2 min_bound = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 max_bound = { min_bound.x + window_size.x, min_bound.y + window_size.y };

        ImVec2 viewport_cursor_pos = ImGui::GetCursorPos();
        ImVec2 viewport_offset = {
          viewport_cursor_pos.x + (viewport_padding.x / 2.f),
          viewport_cursor_pos.y + (viewport_padding.y / 2.f)
        };
        min_bound.x += viewport_offset.x;
        min_bound.y += viewport_offset.y;

        editor.viewport_bounds[0] = { min_bound.x, min_bound.y };
        editor.viewport_bounds[1] = { max_bound.x, max_bound.y };

        editor.current_viewport_size = {
          max_bound.x - min_bound.x,
          max_bound.y - min_bound.y
        };

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          ImVec2 mouse_click = ImGui::GetMousePos();
          editor.last_mouse_viewport_click = {
            mouse_click.x - min_bound.x,
            mouse_click.y - min_bound.y
          };
        }

        if (!ImGui::IsWindowHovered() && !ImGui::IsWindowFocused()) {
          editor.last_mouse_viewport_click = std::nullopt;
        }

        if (viewport == nullptr) {
          ScopedColor red_text(ImGuiCol_Text, ui::theme::red);
          ImGui::Text("Failed to find viewport frame, settings may be corrupt");
        } else {
          ImVec2 cursor_pos = ImGui::GetCursorPos();
          ImVec2 curr_win_size = ImGui::GetContentRegionAvail();

          float aspect_ratio = Renderer::GetWindow()->AspectRatio();
          float ui_aspect_ratio = curr_win_size.x / curr_win_size.y;

          glm::vec2 size = { curr_win_size.x, curr_win_size.y };
          if (ui_aspect_ratio > aspect_ratio) {
            size.x = curr_win_size.y * aspect_ratio;
            viewport_padding = { (curr_win_size.x - size.x) * 0.5f, 0.f };
          } else {
            size.y = curr_win_size.x / aspect_ratio;
            viewport_padding = { 0.f, (curr_win_size.y - size.y) * 0.5f };
          }

          ImTextureID tex_id = (void*)(uintptr_t)viewport->texture;
          ImVec2 img_size = { size.x, size.y };

          ImGui::SetCursorPos({ viewport_padding.x, viewport_padding.y });
          ImGui::Image(tex_id, img_size, ImVec2(0, 1), ImVec2(1, 0));
          ImGui::SetCursorPos(cursor_pos);
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
