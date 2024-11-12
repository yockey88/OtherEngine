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