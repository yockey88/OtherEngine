/**
 * \file tests/stress_test.cpp
 **/
#include "mock_engine.hpp"

#include <SDL_mouse.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#include "core/ref.hpp"
#include "core/rand.hpp"
#include "core/time.hpp"

#include "input/io.hpp"

#include "event/event_queue.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/light_source.hpp"

#include "scene/scene.hpp"

#include "rendering/model_factory.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"

using other::Ref;
using other::NewRef;

using other::AssetHandle;
using other::ModelFactory;
using other::Transform;
using other::StaticMesh;
using other::Scene;
    
// for operator""s
using namespace std::chrono_literals;

class StressTest : public other::EngineTest {
  public:
    void SetUp() override;
    void TearDown() override;

  protected:
    AssetHandle cube_handle;
    Ref<Scene> scene = nullptr;

    const uint32_t num_cubes = 50000; 
    const int32_t max_coord = 100;

    Ref<other::CameraBase> camera;
    Ref<other::SceneRenderer> renderer;
  
    other::time::Timer timer{ 30s };
    other::time::DeltaTime dt;

    glm::vec3 RandomScenePosition() const;
};

/**
 * Debug Mode (50,000 cubes)
 *  FPS = 10 :(
 *
 * Release Mode (50,000 cubes)
 *  FPS = 60 :)
 **/
TEST_F(StressTest , lots_o_cubes) {
  float ts = 0.f;
  dt.Start();

  timer.Start();
  do {
    ts = dt.Get();

    EmptyEventLoop();

    other::DefaultUpdateCamera(camera);

    scene->EarlyUpdate(ts);
    scene->Update(ts);
    scene->LateUpdate(ts);

    renderer->SubmitCamera(camera);
    scene->Render(renderer);  
    renderer->EndScene();

    const auto& frames = renderer->GetRender(); 
    const auto& vp = frames.at(other::kDefaultTargetHash);

    other::Renderer::GetWindow()->Clear();
    other::Renderer::DrawFramebufferToWindow(vp);
    other::UI::BeginFrame();
    ImGui::Begin("Data");
    ImGui::Text("num cubes = %d" , num_cubes);
    ImGui::Text("dt = %.1fms" , ts);
    ImGui::Text("FPS = %.1f" , 1.f / (ts / 1000.f));
    ImGui::End();
    other::UI::EndFrame();
    other::Renderer::GetWindow()->SwapBuffers();

    GLenum err = glGetError();
    ASSERT_EQ(err , GL_NO_ERROR);
  } while (timer.Tick());
}

void StressTest::SetUp() {
  scene = NewRef<Scene>();
  cube_handle = ModelFactory::CreateBox({ 1.f , 1.f , 1.f });

  {
    auto* pl = scene->CreateEntity("point-light");
    auto& light = pl->AddComponent<other::LightSource>();
    light.type = other::POINT_LIGHT_SRC;
    light.pointlight = {
      .position = { 0.f , 3.f , 0.f , 1.f }, 
      .radius = (float)max_coord ,
    };
    pl->UpdateComponent(light);
  }


  other::time::Stopwatch stopwatch;

  stopwatch.Start();
  for (uint32_t i = 0; i < num_cubes; ++i) {
    auto* ent = scene->CreateEntity(other::fmtstr("cube {}" , i));
    auto& mesh = ent->AddComponent<StaticMesh>();
    mesh.handle = cube_handle;
    mesh.material = {};

    auto& transform = ent->GetComponent<Transform>();
    transform.position = RandomScenePosition();
  }
  stopwatch.Stop();
  OE_INFO("Generated {} cubes in {}us" , num_cubes , stopwatch.GetDuration());

  auto win_size = other::Renderer::WindowSize();
  camera = NewRef<other::PerspectiveCamera>(glm::ivec2{ win_size.x , win_size.y });
  camera->SetPosition({ 0.f , 0.f , 3.f });
  
  auto renderer = TEST_ENGINE_ENV()->GetDefaultSceneRenderer(num_cubes);

  SDL_SetRelativeMouseMode(SDL_TRUE);

  scene->Initialize();
  scene->Start();
}

void StressTest::TearDown() {
  scene->Stop();
  scene->Shutdown();
  scene = nullptr;
}

glm::vec3 StressTest::RandomScenePosition() const {
  static other::RandomGenerator<int32_t> generator(-1 * max_coord , max_coord);
  return {
    generator.Next() , 
    generator.Next() , 
    generator.Next()
  };
}
