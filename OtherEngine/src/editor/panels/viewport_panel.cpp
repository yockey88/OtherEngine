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
#include "event/event_queue.hpp"

#include "rendering/pipeline.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "editor/editor_state.hpp"
#include "editor/selection_manager.hpp"

namespace other {

  void ViewportPanel::OnAttach() {
    // EditorState& editor = EditorState::Get();
    // editor.editor_camera = NewRef<CameraBase>();
    // editor.editor_camera->SetProjectionType(CameraBase::ProjectionType::PERSPECTIVE);
    // editor.editor_camera->SetPosition({ 0.f, 0.f, 5.f });
    // editor.editor_camera->SetDirection({ 0.f, 0.f, -1.f });
    // editor.editor_camera->SetUp({ 0.f, 1.f, 0.f });
    // editor.editor_camera->SetFov(45.f);
    // editor.editor_camera->SetNear(0.1f);
    // editor.editor_camera->SetFar(1000.f);

    EditorState& editor = EditorState::Get();
    editor.guizmo_mode = ImGuizmo::MODE::LOCAL;

    EventQueue::RegisterEventDispatcher<KeyPressed>(
      "ViewportPanel--KeyPressed",
      { std::bind_front(&ViewportPanel::HandleKeyPressed, this) }
    );

    last_viewport_size = Renderer::GetWindow()->Size();
  }

  void ViewportPanel::OnUpdate(float dt) {
    EditorState& editor = EditorState::Get();
    if (editor.editor_camera == nullptr) {
      return;
    }

    // calculate zoom / fov / etc....

    editor.editor_camera->SetViewport(last_viewport_size);
  }

  void ViewportPanel::OnRender() {}

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

    window_hovered = ImGui::IsWindowHovered();

    EditorState& editor = EditorState::Get();

    // clang-format off
    open_framebuffer_menu = false;
    ui::MenuBar([&]() {
      ui::Menu(
        "Options",
        ui::MenuItem{ "Framebuffer Options", [&]() { open_framebuffer_menu = true; } }
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

    Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
    OE_ASSERT(renderer != nullptr, "No renderer found in scene");

    RenderFramebufferList(renderer);

    viewport = renderer->GetRender(editor.current_viewport_name);
    if (AppState::Scenes()->HasActiveScene()) {
      if (viewport == nullptr) {
        ScopedColor red_text(ImGuiCol_Text, ui::theme::red);
        ImGui::Text("Failed to find viewport frame, settings may be corrupt");
      } else {
        RenderViewport(viewport);
      }
    }

    ImGui::End();
    return true;
  }

  void ViewportPanel::OnProjectChange(const Ref<Project>& project) {}

  void ViewportPanel::SetSceneContext(const Ref<Scene>& scene) {
    active_scene = scene;
  }

  void ViewportPanel::RenderFramebufferList(Ref<SceneRenderer>& renderer) {
    OE_ASSERT(renderer != nullptr, "No renderer found in scene");
    if (open_framebuffer_menu) {
      ImGui::OpenPopup("Rendered Framebuffers");
    }

    if (ImGui::BeginPopupModal("Rendered Framebuffers")) {
      ui::Button("Close", []() { ImGui::CloseCurrentPopup(); });

      EditorState& editor = EditorState::Get();
      auto& pipelines = renderer->GetPipelines();
      for (auto& [id, pipeline] : pipelines) {
        OE_ASSERT(pipeline != nullptr, "Pipeline is null!");
        if (pipeline->TargetSpec().depth && !(pipeline->TargetSpec().color || pipeline->TargetSpec().stencil)) {
          continue;
        }

        /// TODO: render little preview of framebuffer
        if (ui::BeginTreeNode(pipeline->Name().c_str(), false)) {
          ImGui::PushID(id.Get());
          ImGui::Text("Clear Color: %f, %f, %f, %f", pipeline->TargetSpec().clear_color.r, pipeline->TargetSpec().clear_color.g, pipeline->TargetSpec().clear_color.b, pipeline->TargetSpec().clear_color.a);
          ImGui::Text("Output Size: %d x %d", pipeline->TargetSpec().size.x, pipeline->TargetSpec().size.y);

          ImVec2 size = {
            pipeline->TargetSpec().size.x * 0.25f,
            pipeline->TargetSpec().size.y * 0.25f
          };

          void* texture_id = (void*)(uintptr_t)pipeline->GetOutput()->texture;
          ImGui::Image(texture_id, size, { 0, 1 }, { 1, 0 }, { 1, 1, 1, 1 }, { 0, 0, 1, 1 });

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
  }

  void ViewportPanel::RenderViewport(const Ref<Framebuffer>& viewport) {
    OE_ASSERT(viewport != nullptr, "Attempting to render null viewport!");

    EditorState& editor = EditorState::Get();

    /// menu bar for guizmo tools
    ui::MenuBar([&]() {
      ui::Menu(
        "Tools",
        ui::MenuItem(
          "Translate",
          [&]() { editor.guizmo_op = ImGuizmo::OPERATION::TRANSLATE; }
        ),
        ui::MenuItem("Rotate", [&]() { editor.guizmo_op = ImGuizmo::OPERATION::ROTATE; }),
        ui::MenuItem("Scale", [&]() { editor.guizmo_op = ImGuizmo::OPERATION::SCALE; })
      );

      ui::Menu(
        "Modes",
        ui::MenuItem("Local", [&]() { editor.guizmo_mode = ImGuizmo::MODE::LOCAL; }),
        ui::MenuItem(
          "World",
          [&]() { /* editor.guizmo_mode = ImGuizmo::MODE::WORLD; */ }
        )
      );
    });

    ImVec2 min_bound = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    last_viewport_size = { window_size.x, window_size.y };

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
      if (!ImGui::IsWindowHovered()) {
        editor.trace_mouse_click = false;
      } else {
        editor.trace_mouse_click = true;
      }

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
      ImTextureID tex_id = (void*)(uintptr_t)viewport->texture;
      ImGui::Image(tex_id, window_size, ImVec2(0, 1), ImVec2(1, 0));
    }

    /// only render gixmos if simulating or editing
    if (EditorState::scene_mode == SceneEditorMode::PLAYING || !SelectionManager::HasSelection()) {
      return;
    }

    Entity* selected = SelectionManager::ActiveSelection();
    OE_ASSERT(selected != nullptr, "Active Selection is null!");

    if (selected->HasComponent<LightSource>() && editor.guizmo_op == ImGuizmo::OPERATION::SCALE) {
      return;
    }

    Transform& transform = selected->GetComponent<Transform>();
    transform.CalcMatrix();

    /// have to copy here so that we don't effect the originals
    glm::mat4 view = editor.editor_camera->ViewMatrix();
    glm::mat4 projection = editor.editor_camera->ProjectionMatrix();
    glm::mat4 model = transform.model_transform;

    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 real_pos = {
      window_pos.x - viewport_padding.x,
      window_pos.y - viewport_padding.y
    };

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(real_pos.x, real_pos.y, last_viewport_size.x, last_viewport_size.y);

    if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), editor.guizmo_op, editor.guizmo_mode, glm::value_ptr(model)) &&
        ImGuizmo::IsUsing()) {
      glm::vec3 translation;
      glm::quat rotation;
      glm::vec3 scale;
      DecomposeTransformMatrix(model, translation, rotation, scale);

      switch (editor.guizmo_op) {
        // case ImGuizmo::TRANSLATE_X:
        // case ImGuizmo::TRANSLATE_Y:
        // case ImGuizmo::TRANSLATE_Z:
        case ImGuizmo::TRANSLATE:
          transform.position = translation;
          break;

        // case ImGuizmo::ROTATE_X:
        // case ImGuizmo::ROTATE_Y:
        // case ImGuizmo::ROTATE_Z:
        case ImGuizmo::ROTATE: {
          glm::vec3 original_erot = transform.erotation;

          // Map original rotation to range [-180, 180] which is what ImGuizmo gives us
          original_erot.x = fmodf(original_erot.x + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
          original_erot.y = fmodf(original_erot.y + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
          original_erot.z = fmodf(original_erot.z + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();

          glm::vec3 delta_erot = glm::eulerAngles(rotation) - original_erot;

          // Try to avoid drift due numeric precision
          if (fabs(delta_erot.x) < 0.001) delta_erot.x = 0.0f;
          if (fabs(delta_erot.y) < 0.001) delta_erot.y = 0.0f;
          if (fabs(delta_erot.z) < 0.001) delta_erot.z = 0.0f;

          glm::vec3 new_rotation = transform.erotation + delta_erot;
          transform.erotation = new_rotation;
          transform.qrotation = glm::quat(new_rotation);
        } break;

        // case ImGuizmo::SCALE_X:
        // case ImGuizmo::SCALE_Y:
        // case ImGuizmo::SCALE_Z:
        // case ImGuizmo::SCALE_XU:
        // case ImGuizmo::SCALE_YU:
        // case ImGuizmo::SCALE_ZU:
        // case ImGuizmo::SCALEU:
        case ImGuizmo::SCALE:
          transform.scale = scale;
          break;

        case ImGuizmo::UNIVERSAL:
        case ImGuizmo::BOUNDS:
        case ImGuizmo::ROTATE_SCREEN:
          /// no-op
          break;

        default:
          break;
      }

      transform.CalcMatrix();
    }
  }

  bool ViewportPanel::HandleKeyPressed(KeyPressed& event) {
    EditorState& editor = EditorState::Get();
    if (!window_hovered && editor.scene_mode == SceneEditorMode::STOPPED) {
      return false;
    }

    if (event.Key() == Keyboard::Key::OE_W) {
      editor.guizmo_op = ImGuizmo::OPERATION::TRANSLATE;
      return true;
    }

    if (event.Key() == Keyboard::Key::OE_E) {
      editor.guizmo_op = ImGuizmo::OPERATION::ROTATE;
      return true;
    }

    if (event.Key() == Keyboard::Key::OE_R) {
      editor.guizmo_op = ImGuizmo::OPERATION::SCALE;
      return true;
    }

    return false;
  }

}  // namespace other
