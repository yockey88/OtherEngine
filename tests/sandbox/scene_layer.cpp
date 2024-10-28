/**
 * \file scene_layer.cpp
 **/
#include "scene_layer.hpp"

#include "core/filesystem.hpp"

#include "event/core_events.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"

#include "rendering/renderer.hpp"
#include "scripting/script_engine.hpp"

using namespace other;

void SceneLayer::OnAttach() {
  EventQueue::RegisterEventDispatcher<SceneLoad>(
    "Scene-Load",
    {
      std::bind_front(&SceneLayer::HandleSceneLoad, this),
    }
  );

  EventQueue::RegisterEventDispatcher<KeyPressed>(
    "Scene-Control-Key-Binds",
    {
      [&](KeyPressed& event) -> bool {
        bool handled = false;
        HandleKeyEvent(event, Keyboard::Key::OE_S, [&](KeyPressed& event) {
          AppState::Scenes()->StartScene();
          handled = true;
        });

        HandleKeyEvent(event, Keyboard::Key::OE_P, [&](KeyPressed& event) {
          AppState::Scenes()->StopScene();
          handled = true;
        });

        return handled;
      },
    }
  );

  const Path engine_core_dir = Filesystem::GetEngineCoreDir();
  const Path assets_dir = engine_core_dir / "OtherEngine" / "assets";
  const Path scene_dir = assets_dir / "scenes";

  scenepath = Path("C:/Yock/code/OtherEngine/tests/sandbox") / "test_scene.yscn";
  OE_ASSERT(Filesystem::PathExists(scenepath), "Scene file does not exist : {}", scenepath.string());

  if (!AppState::Scenes()->LoadScene(scenepath)) {
    OE_ERROR("Failed to load scene : {}", scenepath.string());
    EventQueue::PushEvent<ShutdownEvent>({ ExitCode::FAILURE });
    return;
  }
}

void SceneLayer::OnLateUpdate(float dt) {
  bvh->Update();
}

bool SceneLayer::HandleSceneLoad(SceneLoad& event) {
  {
    scene = AppState::Scenes()->GetScene(event.scene_id);
    OE_ASSERT(scene != nullptr, "Scene is null!");

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
  AppState::Scenes()->SetAsActive(scenepath);
  AppState::Scenes()->StartScene();

  return false;
}

void SceneLayer::OnDetach() {
}