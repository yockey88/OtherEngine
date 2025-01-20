/**
 * \file editor/editor_state.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_STATE_HPP
#define OTHER_ENGINE_EDITOR_STATE_HPP

#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>

#include "core/defines.hpp"
#include "core/uuid.hpp"

#include "rendering/camera_base.hpp"

namespace other {

  enum class SceneEditorMode {
    STOPPED,
    FREE_CAMERA,
    PLAYING,
    SIMULATING,

    NUM_SCENE_EDITOR_MODES,
    INVALID_SCENE_EDITOR_MODE = NUM_SCENE_EDITOR_MODES
  };

  struct EditorState {
    static inline SceneEditorMode scene_mode = SceneEditorMode::STOPPED;

    bool trace_mouse_click = false;

    Opt<glm::vec2> last_mouse_viewport_click = std::nullopt;
    glm::vec2 current_viewport_size = { 0.f, 0.f };
    glm::vec2 viewport_bounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };

    Ref<CameraBase> editor_camera = nullptr;
    glm::vec3 stored_camera_position = { 0.f, 0.f, 0.f };
    glm::vec3 stored_camera_direction = { 0.f, 0.f, 0.f };

    ImGuizmo::OPERATION guizmo_op = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE guizmo_mode = ImGuizmo::LOCAL;

    UUID current_viewport_name = FNV("Geometry");
    Opt<UUID> panel_creator_id = std::nullopt;

    static EditorState& Get();
    static void Shutdown();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EDITOR_STATE_HPP
