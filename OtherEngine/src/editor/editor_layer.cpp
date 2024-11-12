/**
 * \file editor/editor_layer.cpp
 **/
#include "editor/editor_layer.hpp"

#include <imgui/imgui.h>

#include "core/config.hpp"
#include "core/config_keys.hpp"
#include "core/logger.hpp"

#include "application/app_state.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "input/mouse.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "scripting/script_engine.hpp"

#include "editor/editor.hpp"
#include "editor/editor_images.hpp"
#include "editor/editor_settings.hpp"

namespace other {

  std::vector<float> fb_verts = {
    1.f, 1.f, 1.f, 1.f,
    -1.f, 1.f, 0.f, 1.f,
    -1.f, -1.f, 0.f, 0.f,
    1.f, -1.f, 1.f, 0.f
  };

  std::vector<uint32_t> fb_indices{ 0, 1, 3, 1, 2, 3 };
  std::vector<uint32_t> fb_layout{ 2, 2 };

  void EditorLayer::OnAttach() {
    /// load editor icons and data
    EditorImages::Initialize();
    Renderer::GetWindow()->ForceResize({ 1920, 1080 });

    editor_camera = NewRef<PerspectiveCamera>(Renderer::WindowSize());
    editor_camera->SetPosition({ 0.f, 0.f, 3.f });
    editor_camera->SetDirection({ 0.f, 0.f, -1.f });
    DefaultUpdateCamera(editor_camera);

    LoadEditorScripts(editor_config);

    editor_scripts.ApiCall("NativeInitialize");
    editor_scripts.ApiCall("OnInitialize");

    editor_scripts.ApiCall("NativeStart");
    editor_scripts.ApiCall("OnStart");

    panel_manager = NewScope<PanelManager>();
    panel_manager->Attach((Editor*)ParentApp(), AppState::ProjectContext(), editor_config);

    current_viewport_size = Renderer::WindowSize();

    EventQueue::RegisterEventDispatcher<KeyPressed>(
      "EditorLayer--KeyPressed",
      { std::bind_front(&EditorLayer::HandleKeyPressed, this) }
    );
  }

  void EditorLayer::OnDetach() {
    editor_scripts.ApiCall("OnStop");
    editor_scripts.ApiCall("NativeStop");
    editor_scripts.ApiCall("OnShutdown");
    editor_scripts.ApiCall("NativeShutdown");
    editor_scripts.ApiCall("OnBehaviorUnload");
    panel_manager->Detach();

    EditorImages::Shutdown();
  }

  void EditorLayer::OnEarlyUpdate(float dt) {
    /// go through and trigger any events to dispatch in the next call
    if (Renderer::IsWindowFocused() && lost_window_focus) {
      lost_window_focus = false;
    } else {
      lost_window_focus = true;
    }

    panel_manager->EarlyUpdate(dt);

    editor_scripts.ApiCall("EarlyUpdate", dt);

    AppState::Scenes()->EarlyUpdateScene(dt);
  }

  void EditorLayer::OnUpdate(float dt) {
    panel_manager->Update(dt);

    /// after all early updates, update client and script
    editor_scripts.ApiCall("Update", dt);

    AppState::Scenes()->UpdateScene(dt);
  }

  void EditorLayer::OnLateUpdate(float dt) {
    panel_manager->LateUpdate(dt);

    /// after all early updates, update client and script
    editor_scripts.ApiCall("LateUpdate", dt);

    AppState::Scenes()->LateUpdateScene(dt);
  }

  void EditorLayer::OnRender() {
    Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
    if (camera_free) {
      DefaultUpdateCamera(editor_camera);
    }
    AppState::Scenes()->GetRenderer()->SubmitCamera(editor_camera);
  }

  void EditorLayer::OnUIRender() {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Reload")) {
          // ReloadScripts();
        }

        if (ImGui::MenuItem("Settings")) {
          LaunchSettingsWindow();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Assets")) {
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Objects")) {
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

#if 1
    if (AppState::Scenes()->HasActiveScene() && ImGui::Begin("Viewport")) {
      Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
      OE_ASSERT(renderer != nullptr, "No renderer found in scene");

      auto& frames = renderer->GetRender();
      viewport = nullptr;

      if (auto itr = frames.find(FNV("Geometry")); itr != frames.end()) {
        viewport = itr->second;
      }

      if (viewport == nullptr) {
        ScopedColor red_text(ImGuiCol_Text, ui::theme::red);
        ImGui::Text("No Viewport Generated");
      } else {
        ImVec2 curr_win_size = ImGui::GetContentRegionAvail();
        current_viewport_size = { curr_win_size.x, curr_win_size.y };

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

        uint32_t tex_id = viewport->texture;
        ImVec2 img_size = { size.x, size.y };
        ImGui::SetCursorPos({ padding.x, padding.y });
        ImGui::Image((void*)(uintptr_t)tex_id, img_size, ImVec2(0, 1), ImVec2(1, 0));
      }

      ImGui::End();
    }

    if (ImGui::Begin("Inspector") /* && scene_manager->HasActiveScene() */) {
      if (!playing && ImGui::Button("Play")) {
        AppState::Scenes()->StartScene();
        playing = true;
      } else if (playing && ImGui::Button("Stop Scene")) {
        AppState::Scenes()->StopScene();
        playing = false;
      }

      ImGui::Text("Camera Position :: [%f , %f , %f]", editor_camera->Position().x, editor_camera->Position().y, editor_camera->Position().z);
      ImGui::Text("Camera Direction :: [%f , %f , %f]", editor_camera->Direction().x, editor_camera->Direction().y, editor_camera->Direction().z);
    }
    ImGui::End();

    editor_scripts.ApiCall("RenderUI");

    panel_manager->RenderUI();
#endif
  }

  void EditorLayer::SaveActiveScene() {
    OE_ASSERT(AppState::Scenes()->ActiveScene() != nullptr, "Attempting to save null scene!");

    auto& scenes = AppState::Scenes();
    bool is_playing = scenes->IsPlaying();

    if (is_playing) {
      scenes->StopScene();
    }

    Path p = scenes->ActiveScene()->path;

    scenes->SaveActiveScene();

    // ParentApp()->UnloadScene();
    // ParentApp()->LoadScene(p);

    if (is_playing) {
      scenes->StartScene();
    }
  }

  void EditorLayer::LoadEditorScripts(const ConfigTable& editor_config) {
    OE_DEBUG("Retrieving editor scripts");
    editor_scripts = ScriptEngine::LoadScriptsFromTable(editor_config, kEditorSection);
  }

  void EditorLayer::LaunchSettingsWindow() {
    Ref<UIWindow> settings_window = NewRef<SettingsWindow>();
    // AppState::PushUIWindow(settings_window);
  }

  bool EditorLayer::HandleKeyPressed(KeyPressed& event) {
    HandleKeyEvent(event, Keyboard::Key::OE_C, [&]() {
      camera_free = !camera_free;
      if (camera_free) {
        Mouse::LockCursor();
      } else {
        Mouse::FreeCursor();
      }
    });

    return event.Key() != Keyboard::Key::OE_C;
  }

}  // namespace other
