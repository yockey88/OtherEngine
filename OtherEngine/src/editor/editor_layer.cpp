/**
 * \file editor/editor_layer.cpp
 **/
#include "editor/editor_layer.hpp"

#include <imgui/imgui.h>

#include "core/logger.hpp"
#include "environment/environment.hpp"
#include "math/vecmath.hpp"

#include "application/app_state.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "event/mouse_events.hpp"
#include "input/mouse.hpp"

#include "ecs/components/light_source.hpp"
#include "scene/bvh.hpp"
#include "scene/scene_manager.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "editor/editor_images.hpp"
#include "editor/editor_state.hpp"
#include "editor/panels/file_editor.hpp"
#include "editor/panels/framebuffer_editor.hpp"
#include "editor/panels/pipeline_creator.hpp"
#include "editor/panels/renderpass_creator.hpp"
#include "editor/panels/scene_renderer_settings.hpp"
#include "editor/panels/shader_creator.hpp"
#include "editor/selection_manager.hpp"

namespace other {
  namespace {

    std::vector<float> fb_verts = {
      1.f, 1.f, 1.f, 1.f,
      -1.f, 1.f, 0.f, 1.f,
      -1.f, -1.f, 0.f, 0.f,
      1.f, -1.f, 1.f, 0.f
    };

    std::vector<uint32_t> fb_indices{ 0, 1, 3, 1, 2, 3 };
    std::vector<uint32_t> fb_layout{ 2, 2 };

  }  // namespace

  void EditorLayer::OnAttach() {
    EventQueue::RegisterEventDispatcher<KeyPressed>(
      "EditorLayer--KeyPressed",
      { std::bind_front(&EditorLayer::HandleKeyPressed, this) }
    );
    EventQueue::RegisterEventDispatcher<MouseButtonPressed>(
      "EditorLayer--MousePressed",
      { std::bind_front(&EditorLayer::HandleMousePressed, this) }
    );
    EventQueue::RegisterEventDispatcher<SceneActivate>(
      "EditorLayer--SceneActivate",
      { std::bind_front(&EditorLayer::HandleSceneActivate, this) }
    );
    EventQueue::RegisterEventDispatcher<SceneUnload>(
      "EditorLayer--SceneUnload",
      { std::bind_front(&EditorLayer::HandleSceneUnload, this) }
    );

    // load editor icons and data
    EditorImages::Initialize();
    Renderer::GetWindow()->ForceResize({ 1920, 1080 });

    EditorState& editor = EditorState::Get();

    auto win_size = Renderer::WindowSize();
    editor.editor_camera = NewRef<PerspectiveCamera>(glm::ivec2{ win_size.x, win_size.y });
    editor.editor_camera->SetPosition({ 0.f, 0.f, 3.f });
    editor.editor_camera->locked = true;
    DefaultUpdateCamera(editor.editor_camera);

    panel_manager = NewScope<PanelManager>();
    panel_manager->Attach(AppState::ProjectContext(), editor_config);
  }

  void EditorLayer::OnDetach() {
    panel_manager->Detach();
    EditorImages::Shutdown();

    EventQueue::UnregisterEventDispatcher("EditorLayer--KeyPressed");
    EventQueue::UnregisterEventDispatcher("EditorLayer--MousePressed");
    EventQueue::UnregisterEventDispatcher("EditorLayer--SceneActivate");
    EventQueue::UnregisterEventDispatcher("EditorLayer--SceneUnload");
  }

  void EditorLayer::OnEarlyUpdate(float dt) {
    /// go through and trigger any events to dispatch in the next call
    if (Renderer::IsWindowFocused() && lost_window_focus) {
      lost_window_focus = false;
    } else {
      lost_window_focus = true;
    }

    panel_manager->EarlyUpdate(dt);
  }

  void EditorLayer::OnUpdate(float dt) {
    panel_manager->Update(dt);
  }

  void EditorLayer::OnLateUpdate(float dt) {
    EditorState& editor = EditorState::Get();
    panel_manager->LateUpdate(dt);
    if (editor.editor_camera == nullptr) {
      return;
    }

    if (EditorState::scene_mode == SceneEditorMode::SIMULATING ||
        EditorState::scene_mode == SceneEditorMode::FREE_CAMERA) {
      DefaultUpdateCamera(editor.editor_camera);
    }
  }

  void EditorLayer::OnRender() {
    EditorState& editor = EditorState::Get();
    panel_manager->Render();

    /// dont trace if no scene or not editing
    if (!AppState::Scenes()->HasActiveScene() ||
        EditorState::scene_mode == SceneEditorMode::PLAYING) {
      return;
    }

    /// submit editor camera for main render,
    Ref<CameraBase> editor_camera = editor.editor_camera;
    AppState::Scenes()->GetRenderer()->SubmitCamera(editor_camera);
  }

  void EditorLayer::OnUIRender() {
    using namespace std::string_view_literals;

    bool scene_active = AppState::Scenes()->HasActiveScene();
    bool render_success = scene_active;
    if (scene_active) {
      Ref<SceneRenderer> scene_renderer = AppState::Scenes()->GetRenderer();
      OE_ASSERT(scene_renderer != nullptr, "No scene renderer found");

      SceneMetadata* active_scene = AppState::Scenes()->ActiveScene();
      OE_ASSERT(active_scene != nullptr, "No active scene found");
      OE_ASSERT(active_scene->scene != nullptr, "No active scene found");

      /// render scene as it is for runtime, this clears the pipelines
      /// TODO: finalize scene and then draw editor information on top
      // bool runtime_frame_success = scene_renderer->Render();
      // scene_renderer->ClearLightEnvironment();

      /// render scene for editor
      if (EditorState::scene_mode != SceneEditorMode::PLAYING) {
        // active_scene->bvh->RenderBounds(scene_renderer);

        if (SelectionManager::HasSelection()) {
          Entity* selected = SelectionManager::ActiveSelection();
          OE_ASSERT(selected != nullptr, "Selected entity is null!");

          // RenderSubmission sub = selected->WireframeSubmission();
          // OE_ASSERT(sub.model != nullptr, "Wireframe model is null!");
          // scene_renderer->SubmitStaticModel(sub);
        }
      }

      render_success = scene_renderer->Render();
    }

    EditorState& editor = EditorState::Get();

    // clang-format off
    ui::MainMenuBar([&]() {
      ui::Menu(
        "File",
        ui::MenuItem{ "Reload"sv, [&]() {} },
        ui::MenuItem{ "Settings"sv, [&]() { LaunchSettingsWindow(); } }
      );

      ui::Menu(
        "Edit",
        ui::MenuItem{ "Undo"sv, [&]() {} },
        ui::MenuItem{ "Redo"sv, [&]() {} }
      );

      ui::Menu(
        "View",
        ui::MenuItem{ "Scene"sv, [&]() {} },
        ui::MenuItem{ "Game"sv, [&]() {} },
        ui::MenuItem{ "Inspector"sv, [&]() {} },
        ui::MenuItem{ "Terminal"sv, [&]() {} }
      );

      ui::Menu(
        "Tools",
        ui::MenuItem{ "Pipeline Creator"sv, [&]() { editor.panel_creator_id = panel_manager->AddPanel("Pipeline-Creator", NewRef<PipelineCreator>()); } },
        ui::MenuItem{ "File Editor"sv, [&]() { editor.panel_creator_id = panel_manager->AddPanel("Shader-Creator", NewRef<FileEditor>()); } },
        ui::MenuItem{ "Renderpass Creator"sv, [&]() { editor.panel_creator_id = panel_manager->AddPanel("Renderpass-Creator", NewRef<RenderpassCreator>()); } },
        ui::MenuItem{ "Shader Creator"sv, [&]() { editor.panel_creator_id = panel_manager->AddPanel("Shader-Creator", NewRef<ShaderCreator>()); } },
        ui::MenuItem{ "Framebuffer Creator"sv , [&]() { editor.panel_creator_id = panel_manager->AddPanel("Framebuffer-Creator", NewRef<FramebufferEditor>()); } }
      );

      ui::Menu(
        "Rendering",
        ui::MenuItem{ "Scene Renderer Settings"sv, [&]() { editor.panel_creator_id = panel_manager->AddPanel("Scene-Renderer-Settings", NewRef<SceneRendererSettings>()); } }
      );

      // ui::Menu("Assets", [&]() {});
      // ui::Menu("Objects", [&]() {});
    });
    // clang-format on

    panel_manager->RenderUI();

    /**
     * save-initial-state : void -> void
            // auto& scenes = AppState::Scenes();
            // SceneMetadata* current_scene = scenes->ActiveScene();
            // OE_ASSERT(current_scene != nullptr, "No active scene found");
            // OE_ASSERT(current_scene->scene != nullptr, "No active scene found");
            // initial_state = SaveStack::RecordState(current_scene->scene);
     *
     **/

    if (ImGui::Begin("Inspector")) {
      if (scene_active && !render_success) {
        ScopedColor err_color(ImGuiCol_Text, ui::theme::red);
        if (ImGui::BeginChild("[ ERROR ]", { 0, 0 }, false, ImGuiWindowFlags_NoScrollbar)) {
          ImGui::Text("Failed to render scene");
          ImGui::EndChild();
        }
      } else if (!scene_active) {
      }

      switch (EditorState::scene_mode) {
        case SceneEditorMode::STOPPED:
          ui::Button("Play", [&]() {
            // save-initial-state
            AppState::Scenes()->StartScene();
            EditorState::scene_mode = SceneEditorMode::PLAYING;

            /// lock editor camera to prevent movement while scene is simulating
            editor.editor_camera->locked = true;
          });

          ui::Button("Simulate", [&]() {
            // save-initial-state
            AppState::Scenes()->StartScene();
            EditorState::scene_mode = SceneEditorMode::SIMULATING;

            /// unlock editor camera to move around simulated scene
            editor.editor_camera->locked = false;
          });
          break;

        case SceneEditorMode::FREE_CAMERA: {
          ScopedColor green_text(ImGuiCol_Text, ui::theme::green);
          ImGui::Text("[Roaming] (Ctrl+C to lock, Ctrl+Shift+C to reset)");
        } break;

        case SceneEditorMode::SIMULATING:
        case SceneEditorMode::PLAYING:
          ui::Button("Stop", [&]() {
            AppState::Scenes()->StopScene();
            EditorState::scene_mode = SceneEditorMode::STOPPED;

            /// lock editor camera to prevent movement while scene is not simulating and camera is not free
            editor.editor_camera->locked = true;

            // auto& scenes = AppState::Scenes();
            // SceneMetadata* current_scene = scenes->ActiveScene();
            // OE_ASSERT(current_scene != nullptr, "No active scene found");
            // OE_ASSERT(current_scene->scene != nullptr, "No active scene found");
            // OE_ASSERT(initial_state.has_value(), "No initial state found");

            // SaveStack::RestoreState(current_scene->scene, initial_state.value());
            // initial_state = std::nullopt;

            // current_scene->scene->EarlyUpdate(0.f);
            // current_scene->scene->Update(0.f);
            // current_scene->scene->LateUpdate(0.f);
          });
          break;

        default: {
          ScopedColor red_text(ImGuiCol_Text, ui::theme::red);
          ImGui::Text("Corrupt Editor State");
        } break;
      }
    }
    ImGui::End();
  }

  void EditorLayer::LaunchSettingsWindow() {
    // Ref<UIWindow> settings_window = NewRef<SettingsWindow>();
    // AppState::PushUIWindow(settings_window);
  }

  Ray EditorLayer::CastRay(Ref<CameraBase>& camera, const glm::vec2& mouse_pos) {
    OE_ASSERT(camera != nullptr, "Camera is null!");

    glm::vec4 clip_pos = {
      mouse_pos.x,
      mouse_pos.y,
      -1.f, 1.f
    };

    glm::mat4 inverse_proj = glm::inverse(camera->ProjectionMatrix());
    glm::mat4 inverse_camera_tansform = glm::inverse(camera->ViewMatrix());
    glm::vec4 ray_dir = inverse_camera_tansform * inverse_proj * clip_pos;

    return Ray(camera->Position(), glm::normalize(glm::vec3(ray_dir)));
  }

  bool EditorLayer::HandleKeyPressed(KeyPressed& event) {
    if (event.Key() == Keyboard::Key::OE_ESCAPE) {
      if (EditorState::scene_mode == SceneEditorMode::PLAYING ||
          EditorState::scene_mode == SceneEditorMode::SIMULATING) {
        AppState::Scenes()->StopScene();
        playing = false;
        Mouse::FreeCursor();
        EditorState::scene_mode = SceneEditorMode::STOPPED;

        /// FIXME: theres a bug here in which cameras are being updated
        ///         and when, need to update cameras with these rules:
        ///       - 1 : if scene stopped, no update any camera, render editor camera
        ///       - 2 : if scene playing, only update active scene camera-entity,
        ///               and don't submit editor camera to renderer
        ///       - 3 : if simulating, update editor camera, and only update scene cameras
        ///               based on user-independent data (if they have something moving the camera based
        ///               off user input, turn it off) then render through editor camera

        // editor_camera->locked = !editor_camera->locked;
        // editor_camera->locked ?
        //   Mouse::FreeCursor() :
        //   Mouse::LockCursor();

        return true;
      }

      /// dont interrupt escape key if we're not playing
      return false;
    }

    if (event.Key() == Keyboard::Key::OE_F1) {
      Environment::Get().terminal_open = !Environment::Get().terminal_open;
      return true;
    }

    if (event.Key() == Keyboard::Key::OE_RETURN) {
      if (Environment::Get().terminal_open) {
        Environment::ReadTerminalInput();
        return true;
      }
    }

    return HandleCtrlLayerKeyEvent(event, Keyboard::Key::OE_C, [&]() -> bool {
      if (EditorState::scene_mode == SceneEditorMode::PLAYING ||
          EditorState::scene_mode == SceneEditorMode::SIMULATING) {
        return false;
      }

      EditorState& editor = EditorState::Get();

      editor.editor_camera->locked = !editor.editor_camera->locked;
      editor.editor_camera->locked ?
        Mouse::FreeCursor() :
        Mouse::LockCursor();

      if (!editor.editor_camera->locked) {
        EditorState::scene_mode = SceneEditorMode::FREE_CAMERA;

        if (Keyboard::LCtrlShiftLayer()) {
          editor.stored_camera_position = editor.editor_camera->Position();
          editor.stored_camera_direction = editor.editor_camera->Direction();
        }
      } else {
        EditorState::scene_mode = SceneEditorMode::STOPPED;

        if (Keyboard::LCtrlShiftLayer()) {
          editor.editor_camera->SetPosition(editor.stored_camera_position);
          editor.editor_camera->SetDirection(editor.stored_camera_direction);
          editor.editor_camera->CalculateMatrix();
        }
      }

      return true;
    });
  }

  bool EditorLayer::HandleMousePressed(MouseButtonPressed& event) {
    if (!EditorState::Get().last_mouse_viewport_click.has_value()) {
      return false;
    }

    EditorState& editor = EditorState::Get();
    if (EditorState::scene_mode != SceneEditorMode::STOPPED ||
        event.button != Mouse::Button::LEFT) {
      return false;
    }

    if (!editor.trace_mouse_click) {
      return false;
    }

    SceneMetadata* scene = AppState::Scenes()->ActiveScene();
    OE_ASSERT(scene != nullptr, "No active scene!");
    OE_ASSERT(scene->scene != nullptr, "Scene is null!");
    OE_ASSERT(scene->bvh != nullptr, "Scene BVH is null!");

    glm::vec4 homogeneous_clip_pos = {
      (2.f * editor.last_mouse_viewport_click->x) / editor.current_viewport_size.x - 1.f,
      1.f - (2.f * editor.last_mouse_viewport_click->y) / editor.current_viewport_size.y,
      -1.f, 1.f
    };
    glm::vec4 shift_pos = glm::vec4(glm::vec2(glm::inverse(editor.editor_camera->ProjectionMatrix()) * homogeneous_clip_pos), -1.f, 0.f);
    glm::vec3 world_coords = glm::vec3(glm::normalize(glm::inverse(editor.editor_camera->ViewMatrix()) * shift_pos));

    Ray initial_ray = Ray{
      editor.editor_camera->Position(),
      world_coords
    };
    Interval trace_interval = Interval(0.f, max_value<float>());
    Opt<TraceResult> trace = scene->bvh->Trace(initial_ray, trace_interval);
    if (!trace.has_value()) {
      return false;
    }

    OE_ASSERT(trace->hit_entity != nullptr, "Hit entity is null!");
    if (trace->hit_entity->HasComponent<LightSource>()) {
      const LightSource& light = trace->hit_entity->ReadComponent<LightSource>();
      if (light.type == LightSourceType::DIRECTION_LIGHT_SRC) {
        editor.guizmo_op = ImGuizmo::OPERATION::ROTATE;
      }
    }

    SelectionManager::Select(trace->hit_entity);
    return false;
  }

  bool EditorLayer::HandleSceneActivate(SceneActivate& event) {
    OE_ASSERT(AppState::Scenes()->HasActiveScene(), "No active scene found");
    OE_ASSERT(panel_manager != nullptr, "Panel manager is null");

    SceneMetadata* current_scene = AppState::Scenes()->ActiveScene();
    OE_ASSERT(current_scene != nullptr, "No active scene found");
    OE_ASSERT(current_scene->scene != nullptr, "No active scene found");

    AppState::AppEvent(EngineStateEvent::EDIT_SCENE);
    panel_manager->OnSceneActivate(current_scene);

    /// dont interrupt event
    return false;
  }

  bool EditorLayer::HandleSceneUnload(SceneUnload& event) {
    AppState::AppEvent(EngineStateEvent::SCENE_UNLOADED);

    OE_ASSERT(panel_manager != nullptr, "Panel manager is null");
    panel_manager->OnSceneDeactivate();

    /// dont interrupt event
    return false;
  }

}  // namespace other
