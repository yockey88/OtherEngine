/**
 * \file editor/panels/viewport_panel.cpp
 **/
#include "editor/panels/viewport_panel.hpp"

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imguizmo/ImGuizmo.h>

#include "math/matrix_math.hpp"

#include "application/app_state.hpp"

#include "rendering/pipeline.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "editor/editor_settings.hpp"
#include "editor/editor_state.hpp"
#include "editor/selection_manager.hpp"

namespace other {

  void ViewportPanel::OnRender() {
  }

  bool ViewportPanel::OnGuiRender(bool& is_open) {
    if (!is_open) {
      return false;
    }
    // clang-format off
    if (!ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | 
                                           ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
      // clang-format on
      ImGui::End();
      return false;
    }

    EditorState& editor = EditorState::Get();

    // clang-format off
    bool open_framebuffer_menu = false;
    ui::MenuBar([&]() {
      ui::Menu(
        "Options",
        ui::MenuItem{ "Change Final Framebuffer", [&]() { open_framebuffer_menu = true; } }
      );

      ui::Menu(
        "Tools",
        ui::MenuItem{ "Translate", [&]() { editor.guizmo_op = ImGuizmo::OPERATION::TRANSLATE; } },
        ui::MenuItem{ "Rotate", [&]() { editor.guizmo_op = ImGuizmo::OPERATION::ROTATE; } },
        ui::MenuItem{ "Scale", [&]() { editor.guizmo_op = ImGuizmo::OPERATION::SCALE; } }
      );

      ui::Menu(
        "Modes",
        ui::MenuItem{ "Local", [&]() { editor.guizmo_mode = ImGuizmo::MODE::LOCAL; } },
        ui::MenuItem{ "World", [&]() { /* editor.guizmo_mode = ImGuizmo::MODE::WORLD; */ } }
      );
    });
    // clang-format on

    if (open_framebuffer_menu) {
      ImGui::OpenPopup("Rendered Framebuffers");
    }
    if (ImGui::BeginPopupModal("Rendered Framebuffers")) {
      Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
      OE_ASSERT(renderer != nullptr, "No renderer found in scene");

      auto& pipelines = renderer->GetPipelines();
      for (const auto& [id, pipeline] : pipelines) {
        /// TODO: render little preview of framebuffer
        if (ui::BeginTreeNode(pipeline->Name().c_str(), false)) {
          ImGui::PushID(id.Get());
          ImGui::Text("Clear Color: %f, %f, %f, %f", pipeline->TargetSpec().clear_color.r, pipeline->TargetSpec().clear_color.g, pipeline->TargetSpec().clear_color.b, pipeline->TargetSpec().clear_color.a);
          ImGui::Text("Output Size: %d x %d", pipeline->TargetSpec().size.x, pipeline->TargetSpec().size.y);

          ui::Button("Set Active", [&]() {
            editor.current_viewport_name = id;
            ImGui::CloseCurrentPopup();
          });

          ImGui::PopID();
          ui::EndTreeNode();
        }
      }

      ImGui::EndPopup();
    }

    if (AppState::Scenes()->HasActiveScene()) {
      EditorState& editor = EditorState::Get();

      Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
      OE_ASSERT(renderer != nullptr, "No renderer found in scene");

      /// FIXME: frame is hardcodede
      auto& frames = renderer->GetRender();
      Ref<Framebuffer> viewport = nullptr;
      auto itr = frames.find(editor.current_viewport_name);
      if (itr == frames.end()) {
        // default to geometry frame
        itr = frames.find(FNV("Geometry"));
      }
      viewport = itr->second;

      RenderViewport(viewport);
    }

    ImGui::End();
    return true;
  }

  void ViewportPanel::OnProjectChange(const Ref<Project>& project) {
  }

  void ViewportPanel::SetSceneContext(const Ref<Scene>& scene) {
    active_scene = scene;
  }

  void ViewportPanel::RenderViewport(const Ref<Framebuffer>& viewport) {
    OE_ASSERT(viewport != nullptr, "Attempting to render null viewport!");

    EditorState& editor = EditorState::Get();

    /// menu bar for guizmo tools
    ui::MenuBar([&]() {
      ui::Menu(
        "Tools",
        ui::MenuItem("Translate", [&]() { editor.guizmo_op = ImGuizmo::OPERATION::TRANSLATE; }),
        ui::MenuItem("Rotate", [&]() { editor.guizmo_op = ImGuizmo::OPERATION::ROTATE; }),
        ui::MenuItem("Scale", [&]() { editor.guizmo_op = ImGuizmo::OPERATION::SCALE; })
      );

      ui::Menu(
        "Modes",
        ui::MenuItem("Local", [&]() { editor.guizmo_mode = ImGuizmo::MODE::LOCAL; }),
        ui::MenuItem("World", [&]() { /* editor.guizmo_mode = ImGuizmo::MODE::WORLD; */ })
      );
    });

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

    /// only render gixmos if simulating or editing
    if (EditorState::scene_mode == SceneEditorMode::PLAYING ||
        !SelectionManager::HasSelection()) {
      return;
    }
    Entity* selected = SelectionManager::ActiveSelection();
    OE_ASSERT(selected != nullptr, "Active Selection is null!");

    Transform& transform = selected->GetComponent<Transform>();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    ImVec2 window_pos = ImGui::GetWindowPos();
    ImGuizmo::SetRect(window_pos.x, window_pos.y, window_size.x, window_size.y);

    /// have to copy here so that we don't effect the originals
    glm::mat4 view = editor.editor_camera->ViewMatrix();
    glm::mat4 projection = editor.editor_camera->ProjectionMatrix();
    glm::mat4 model = transform.model_transform;

    // clang-format off
    if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), editor.guizmo_op, editor.guizmo_mode,
        glm::value_ptr(model), nullptr, nullptr)) {
      // clang-format on

      glm::vec3 translation;
      glm::quat rotation;
      glm::vec3 scale;
      DecomposeTransformMatrix(model, translation, rotation, scale);

      switch (editor.guizmo_op) {
        case ImGuizmo::TRANSLATE_X:
        case ImGuizmo::TRANSLATE_Y:
        case ImGuizmo::TRANSLATE_Z:
        case ImGuizmo::TRANSLATE:
          transform.position = translation;
          break;

        case ImGuizmo::ROTATE_X:
        case ImGuizmo::ROTATE_Y:
        case ImGuizmo::ROTATE_Z:
        case ImGuizmo::ROTATE:
          transform.qrotation = rotation;
          transform.erotation = glm::eulerAngles(rotation);
          break;

        case ImGuizmo::SCALE_X:
        case ImGuizmo::SCALE_Y:
        case ImGuizmo::SCALE_Z:
        case ImGuizmo::SCALE_XU:
        case ImGuizmo::SCALE_YU:
        case ImGuizmo::SCALE_ZU:
        case ImGuizmo::SCALEU:
        case ImGuizmo::SCALE:
          transform.scale = scale;
          break;

        case ImGuizmo::UNIVERSAL:
        case ImGuizmo::BOUNDS:
        case ImGuizmo::ROTATE_SCREEN:
          /// no-op
          break;
      }

      transform.CalcMatrix();
    }
  }

}  // namespace other
