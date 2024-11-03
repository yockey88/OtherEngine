/**
 * \file scene_layer.cpp
 **/
#include "scene_layer.hpp"

#include "event/event_queue.hpp"
#include "event/key_events.hpp"

#include "rendering/renderer.hpp"
#include "scripting/script_engine.hpp"

using namespace other;

void SceneLayer::OnAttach() {
  EventQueue::RegisterEventDispatcher<SceneLoad>(
    "SceneLayer--SceneLoad",
    { std::bind_front(&SceneLayer::HandleSceneLoad, this) }
  );

  // EventQueue::RegisterEventDispatcher<KeyPressed>(
  //   "Scene-Control-Key-Binds",
  //   {
  //     [&](KeyPressed& event) -> bool {
  //       bool handled = false;
  //       HandleKeyEvent(event, Keyboard::Key::OE_S, [&]() {
  //         AppState::Scenes()->StartScene();
  //         handled = true;
  //       });

  //       HandleKeyEvent(event, Keyboard::Key::OE_P, [&]() {
  //         AppState::Scenes()->StopScene();
  //         handled = true;
  //       });

  //       return handled;
  //     },
  //   }
  // );
}

void SceneLayer::OnLateUpdate(float dt) {
  bvh->Update();
}

bool SceneLayer::HandleSceneLoad(SceneLoad& event) {
  {
    SceneMetadata* active_scene = AppState::Scenes()->ActiveScene();
    OE_ASSERT(active_scene != nullptr, "No active scene to attach");
    OE_ASSERT(active_scene->scene != nullptr, "Active scene has no scene to attach");
    scene = active_scene->scene;

    auto* cube = scene->GetEntity("cube");
    auto* floor = scene->GetEntity("floor");
    auto* sun = scene->GetEntity("sun");

    auto& cube_mesh = cube->GetComponent<StaticMesh>();
    cube_mesh.material = cube_material1;

    auto& floor_mesh = floor->GetComponent<StaticMesh>();
    floor_mesh.material = cube_material2;

    auto& sun_l = sun->GetComponent<LightSource>();
    sun_l.direction_light = {
      .direction = { 0.f, -1.f, 0.f, 1.f },
      .color = { 0.22f, 0.22f, 0.11f, 1.f },
    };

    bvh = NewRef<BvhTree>(glm::vec3{ 0.f, 0.f, 0.f });
    bvh->AddScene(scene, glm::zero<glm::vec3>());
    OE_DEBUG("Scene loaded {}", scenepath.string());

    ScriptEngine::SetSceneContext(scene);
    Renderer::SetSceneContext(scene);
  }

  return false;
}

void SceneLayer::OnDetach() {
}