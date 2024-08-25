/**
 * \file tests/stress_test.cpp
 **/
#include "mock_engine.hpp"

#include <chrono>
#include <core/time.hpp>
#include <ecs/components/light_source.hpp>

#include "core/ref.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/camera.hpp"

#include "scene/scene.hpp"

#include "rendering/model_factory.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"

using other::Ref;
using other::NewRef;

using other::AssetHandle;
using other::ModelFactory;
using other::StaticMesh;
using other::Scene;

class StressTest : public other::EngineTest {
  public:
    void SetUp() override {
      scene = NewRef<Scene>();
      cube_handle = ModelFactory::CreateBox({ 1.f , 1.f , 1.f });

      {
        auto* pl = scene->CreateEntity("point-light");
        auto& light = pl->AddComponent<other::LightSource>();
        light.type = other::POINT_LIGHT_SRC;
        light.pointlight = {
          .position = { 0.f , 3.f , 0.f , 1.f }, 
        };
        pl->UpdateComponent(light);
      }

      const uint32_t num_cubes = 1; // 0000;

      auto before = std::chrono::steady_clock::now();
      for (uint32_t i = 0; i < num_cubes; ++i) {
        auto* ent = scene->CreateEntity(other::fmtstr("cube {}" , i));
        auto& mesh = ent->AddComponent<StaticMesh>();
        mesh.handle = cube_handle;
        mesh.material = {};
      }
      auto after = std::chrono::steady_clock::now();
      auto dur = after - before;
      OE_INFO("Generated {} cubes in {} millisecond" , num_cubes , std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());

      scene->Initialize();
    }

    void TearDown() override {
      scene->Shutdown();
      scene = nullptr;
    }

  protected:
    AssetHandle cube_handle;
    Ref<Scene> scene = nullptr;
};

TEST_F(StressTest , cubes_10_000) {
  using namespace std::chrono_literals;

  auto renderer = TEST_ENGINE_ENV()->GetDefaultSceneRenderer();

  other::time::Timer timer(10s);
  other::time::FrameRateEnforcer<60> fps;
      
  auto win_size = other::Renderer::WindowSize();
  Ref<other::CameraBase> camera = NewRef<other::PerspectiveCamera>(glm::ivec2{ win_size.x , win_size.y });
  camera->SetPosition({ 0.f , 0.f , 3.f });

  scene->Start();
  timer.Start();
  do {
    auto ts = fps.TimeStep();

    scene->EarlyUpdate(ts);
    scene->Update(ts);
    scene->LateUpdate(ts);

    renderer->SubmitCamera(camera);
    scene->Render(renderer);  
    renderer->EndScene();

    const auto& frames = renderer->GetRender(); 
    const auto& vp = frames.at(other::kDefaultTargetHash);

    other::Renderer::DrawFramebufferToWindow(vp);
     
    fps.Enforce();
  } while (timer.Tick());
  scene->Stop();
}
