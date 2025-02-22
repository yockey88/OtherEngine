/**
 * \file editor/editor_layer.cpp
 **/
#include "editor/editor_layer.hpp"

#include <imgui/imgui.h>

#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "environment/environment.hpp"
#include "math/vecmath.hpp"

#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "event/mouse_events.hpp"
#include "input/mouse.hpp"

#include "ecs/components/light_source.hpp"
#include "scene/bvh.hpp"
#include "scene/scene_manager.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"
#include "rendering/shader.hpp"
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

    std::vector<float> grid_vertices = {
      1.f, 1.f, 1.f, 1.f,
      -1.f, 1.f, 0.f, 1.f,
      -1.f, -1.f, 0.f, 0.f,
      1.f, -1.f, 1.f, 0.f
    };

    std::vector<uint32_t> grid_indices{ 0, 1, 3, 1, 2, 3 };
    std::vector<uint32_t> grid_layout{ 2, 2 };

    std::vector<float> frustum_vertices = {
      -1.f, -1.f, 1.f,
      1.f, -1.f, 1.f,
      1.f, 1.f, 1.f,
      -1.f, 1.f, 1.f,
      -1.f, -1.f, -1.f,
      1.f, -1.f, -1.f,
      1.f, 1.f, -1.f,
      -1.f, 1.f, -1.f
    };

    std::vector<uint32_t> frustum_indices = {
      0, 1,  // (top-front)
      1,     // (top-left.1)
      5,     // (top-left.2)
      5, 4,  // (top-back)
      4,
      0,  // (top-right)
      /// bottom face
      3,     // (bottom-front.1)
      2,     // (bottom-front.2)
      2, 6,  // (bottom-left)
      6,
      7,  // (bottom-back)
      7,  // (bottom-right.1)
      3,  // (bottom-right.2)
      /// front face
      0, 3,  // (front-right)
      1,
      2,  // (front-left)
      /// left face
      5,  // (back-left.1)
      6,  // (back-left.2)
      /// back face
      4, 7  // (back-right)
    };
    std::vector<uint32_t> frustum_layout = { 3 };

    constexpr std::string_view kEditorGridShader = "editor_grid";
    constexpr std::string_view kCameraFrustumShader = "camera_frustum";

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
    EventQueue::RegisterEventDispatcher<MouseButtonHeld>(
      "EditorLayer--MouseHeld",
      { std::bind_front(&EditorLayer::HandleMouseHeld, this) }
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
    // Renderer::GetWindow()->ForceResize({ 1920, 1080 });

    EditorState& editor = EditorState::Get();

    auto win_size = Renderer::WindowSize();
    editor.editor_camera = NewRef<PerspectiveCamera>(glm::ivec2{ win_size.x, win_size.y });
    editor.editor_camera->SetPosition({ 0.f, 3.f, 3.f });
    editor.editor_camera->locked = true;
    DefaultUpdateCamera(editor.editor_camera);

    panel_manager = NewScope<PanelManager>();
    panel_manager->Attach(AppState::ProjectContext(), editor_config);

    editor_grid_vao = NewRef<VertexArray>(grid_vertices, grid_indices, grid_layout);
    OE_ASSERT(editor_grid_vao != nullptr, "Failed to create editor grid vao");

    camera_frustum_vao = NewRef<VertexArray>(frustum_vertices, frustum_indices, frustum_layout);
    OE_ASSERT(camera_frustum_vao != nullptr, "Failed to create camera frustum vao");

    {
      Ref<Directory> shaders = Filesystem::GetDirectory("shaders");
      OE_ASSERT(shaders != nullptr, "Failed to retrieve shaders directory");

      Ref<FileHandle> grid_shader_file = shaders->GetFileHandleByName(kEditorGridShader, ".oshader");
      OE_ASSERT(grid_shader_file != nullptr, "Failed to get shader file : {}", kCameraFrustumShader);
      Ref<FileHandle> frustum_shader_file = shaders->GetFileHandleByName(kCameraFrustumShader, ".oshader");
      OE_ASSERT(frustum_shader_file != nullptr, "Failed to get shader file : {}", kEditorGridShader);

      editor_grid_shader = BuildShader(Path(*grid_shader_file));
      OE_ASSERT(editor_grid_shader != nullptr, "Failed to build editor grid shader");
      camera_frustum_shader = BuildShader(Path(*frustum_shader_file));
      OE_ASSERT(camera_frustum_shader != nullptr, "Failed to build camera frustum shader");
    }

    editor_grid_transform.qrotation = glm::quat(glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));
    editor_grid_transform.erotation = glm::eulerAngles(editor_grid_transform.qrotation);
    editor_grid_transform.scale = glm::vec3(10000.f);
    editor_grid_transform.position = glm::vec3(0.f);
    editor_grid_transform.CalcMatrix();
  }

  void EditorLayer::OnDetach() {
    panel_manager->Detach();
    EditorImages::Shutdown();
    EditorState::Shutdown();

    EventQueue::UnregisterEventDispatcher("EditorLayer--KeyPressed");
    EventQueue::UnregisterEventDispatcher("EditorLayer--MousePressed");
    EventQueue::UnregisterEventDispatcher("EditorLayer--MouseHeld");
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

    if (editor.selected_camera != nullptr) {
      // editor.se
    }

    if (editor.editor_camera == nullptr) {
      return;
    }

    if (EditorState::scene_mode == SceneEditorMode::FREE_CAMERA) {
      DefaultUpdateCamera(editor.editor_camera);
    }
  }

  void EditorLayer::OnRender() {
    EditorState& editor = EditorState::Get();
    panel_manager->Render();

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
    EditorState& editor = EditorState::Get();

    bool scene_active = AppState::Scenes()->HasActiveScene();
    bool render_success = scene_active;
    if (scene_active) {
      Ref<SceneRenderer> scene_renderer = AppState::Scenes()->GetRenderer();
      OE_ASSERT(scene_renderer != nullptr, "No scene renderer found");

      SceneMetadata* active_scene = AppState::Scenes()->ActiveScene();
      OE_ASSERT(active_scene != nullptr, "No active scene found");
      OE_ASSERT(active_scene->scene != nullptr, "No active scene found");

      /// TODO: finalize scene and then draw editor information on top
      /// render scene as it is for runtime, this clears the pipelines
      // bool runtime_frame_success = scene_renderer->Render();
      // scene_renderer->ClearLightEnvironment();

      /// render scene for editor
      if (EditorState::scene_mode != SceneEditorMode::PLAYING) {
        /// editor grid draw command
        scene_renderer->SubmitDebugDrawCommands(
          "Geometry",
          {
            [&]() {
              OE_ASSERT(editor_grid_shader != nullptr, "Editor grid shader is null");
              editor_grid_shader->Bind();
              editor_grid_shader->SetUniform("model", editor_grid_transform.model_transform);
              editor_grid_vao->Draw(DrawMode::TRIANGLES);
              editor_grid_shader->Unbind();
            },
          }
        );

        scene_renderer->SubmitDebugDrawCommands(
          "Geometry",
          {
            [&]() {
              OE_ASSERT(camera_frustum_shader != nullptr, "Camera frustum shader is null");
              if (!editor.camera_selected) {
                return;
              }
              OE_ASSERT(editor.selected_camera != nullptr, "Selected camera is null");

              camera_frustum_shader->Bind();
              camera_frustum_shader->SetUniform("selected_clip_bounds", editor.selected_camera->Clip());
              camera_frustum_shader->SetUniform("selected_camera_pos", editor.selected_camera->Position());
              camera_frustum_shader->SetUniform("selected_camera_inv_mvp", editor.selected_camera->InverseMatrix());
              camera_frustum_vao->Draw(DrawMode::LINES);
              camera_frustum_shader->Unbind();
            },
          }
        );

        Ref<PhysicsWorld> physics_world = active_scene->scene->GetPhysicsWorld();
        physics_world->SubmitRaycastDrawCommands(scene_renderer);
      }

      if (rendering_physics_colliders) {
        active_scene->scene->RenderPhysicsDebug(scene_renderer);
      }

      render_success = scene_renderer->Render();
    }

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

      ui::Menu(
        "Scene",
        ui::MenuItem{ "Show Colliders"sv, [&]() { 
          rendering_physics_colliders = !rendering_physics_colliders; 
          AppState::Scenes()->SetDebugPhysicsRendering(!AppState::Scenes()->IsDebugPhysicsRendering());
        } }
      );
    });
    // clang-format on

    bool ui_signal = false;
    ui_signal = panel_manager->RenderUI();
    // if (EditorState::scene_mode != SceneEditorMode::PLAYING) {
    // }
    /// do something with the fact that a UI panel was interacted with????
    ///   this may not need to return a bool at all

    bool open = true;
    if (ImGui::Begin("Inspector", &open, ImGuiWindowFlags_MenuBar)) {
      ui::MenuBar([&]() {
        static ByteBuffer buffer;
        ui::Menu(
          "File",
          ui::MenuItem("Save Scene", [&]() {
            SceneMetadata* active_scene = AppState::Scenes()->ActiveScene();
            OE_ASSERT(active_scene != nullptr, "No active scene found");
            OE_ASSERT(active_scene->scene != nullptr, "No active scene found");

            SceneSerializer::Write(buffer, active_scene->scene);
            OE_TRACE("Scene Buffer : {}", buffer.DumpBuffer());
            /// FIXME: make this work
            {
              Ref<Directory> scene_dir = Filesystem::GetDirectory("scenes");
              Path scene_path = Path(*scene_dir) / "forest2.oscn";
              std::ofstream scene_file(scene_path.c_str(), std::ios::binary);
              if (scene_file.is_open()) {
                scene_file.write((const char*)buffer.RawBytes(), buffer.Size());
                scene_file.close();
              } else {
                OE_ERROR("Failed to open scene file for writing : {}", scene_path);
              }
            }
          }),
          ui::MenuItem("Load Scene", [&]() {
            SceneMetadata* active_scene = AppState::Scenes()->ActiveScene();
            OE_ASSERT(active_scene != nullptr, "No active scene found");
            OE_ASSERT(active_scene->scene != nullptr, "No active scene found");

            ByteBuffer scene_buffer;
            /// FIXME: make this work too
            {
              Ref<Directory> scene_dir = Filesystem::GetDirectory("scenes");
              Path scene_path = Path(*scene_dir) / "forest2.oscn";
              std::ifstream scene_file(scene_path.c_str(), std::ios::binary);

              if (scene_file.is_open()) {
                scene_file.seekg(0, std::ios::end);
                size_t size = scene_file.tellg();
                scene_file.seekg(0, std::ios::beg);

                std::vector<uint8_t> buffer;
                buffer.resize(size);
                scene_file.read((char*)buffer.data(), size);

                scene_buffer.Write(buffer.data(), buffer.size());
                scene_file.close();
              } else {
                OE_ERROR("Failed to open scene file for reading : {}", scene_path);
              }
            }
            if (scene_buffer.Empty()) {
              return;
            }
            OE_TRACE("Scene Buffer : {}", scene_buffer.DumpBuffer());
            SceneSerializer::Read(scene_buffer, active_scene->scene);
          })
        );
      });

      if (scene_active && !render_success) {
        ScopedColor err_color(ImGuiCol_Text, ui::theme::red);
        if (ImGui::BeginChild("[ ERROR ]", { 0, 0 }, false, ImGuiWindowFlags_NoScrollbar)) {
          ImGui::Text("Failed to render scene");
          ImGui::EndChild();
        }
      } else if (!scene_active) {
        ImGui::End();
        return;
      }

      /// TODO: fix screen rendering!!!!!
      // if (EditorState::scene_mode == SceneEditorMode::PLAYING) {
      //   /// draw scene to screen
      //   Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
      //   OE_ASSERT(renderer != nullptr, "No renderer found in scene");

      //   Ref<Framebuffer> viewport = renderer->GetRender(editor.current_viewport_name);
      //   OE_ASSERT(viewport != nullptr, "Failed to find viewport frame, settings may be corrupt");
      //   Renderer::DrawFramebufferToWindow(viewport);
      // }

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
            {
              SceneMetadata* active_scene = AppState::Scenes()->ActiveScene();
              OE_ASSERT(active_scene != nullptr, "No active scene found");
              OE_ASSERT(active_scene->scene != nullptr, "No active scene found");
              active_scene->scene->Synchronize();
            }
            EditorState::scene_mode = SceneEditorMode::STOPPED;

            /// lock editor camera to prevent movement while scene is not simulating and camera is not free
            editor.editor_camera->locked = true;
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

  bool EditorLayer::HandleMouseHeld(MouseButtonHeld& event) {
    if (EditorState::scene_mode != SceneEditorMode::STOPPED) {
      return false;
    }

    if (event.Button() != Mouse::Button::MIDDLE) {
      return false;
    }

    EditorState& editor = EditorState::Get();
    if (SelectionManager::HasSelection()) {
      /// move selected entity
    } else {
      /// move camera
      glm::vec4 homogeneous_clip_pos = {
        (2.f * Mouse::GetPos().x) / editor.current_viewport_size.x - 1.f,
        1.f - (2.f * Mouse::GetPos().y) / editor.current_viewport_size.y,
        -1.f, 1.f
      };
      glm::vec4 shift_pos = glm::vec4(glm::vec2(glm::inverse(editor.editor_camera->ProjectionMatrix()) * homogeneous_clip_pos), -1.f, 0.f);
      glm::vec3 world_coords_direction = glm::vec3(glm::normalize(glm::inverse(editor.editor_camera->ViewMatrix()) * shift_pos));
      editor.editor_camera->SetTarget(world_coords_direction);
      editor.editor_camera->CalculateMatrix();
    }
    return true;
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

  bool EditorLayer::HandleFileModified(FileModified& event) {
    OE_DEBUG("Modified file : {}", event.handle);
    Ref<FileHandle> file = Filesystem::GetFile(event.handle);
    OE_ASSERT(file != nullptr, "Failed to get file handle from event");
    OE_DEBUG("  > file name : {}", file->FileName());

    return false;
  }

}  // namespace other
