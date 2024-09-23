/**
 * \file tests/stress_test.cpp
 **/
#include "mock_engine.hpp"

#include <imgui/imgui.h>
#include <mutex>
#include <thread>

#include <SDL_mouse.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#include "core/ref.hpp"
#include "core/rand.hpp"
#include "core/time.hpp"

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

    void LoadCubes();

  protected:
    AssetHandle cube_handle;
    Ref<Scene> scene = nullptr;
    bool test_running = false;

    const uint32_t num_cubes = 5000; 
    const int32_t max_coord = 100;

    Ref<other::CameraBase> camera;
    Ref<other::SceneRenderer> renderer;
  
    uint64_t load_time_us;
    other::time::Timer timer{ 30s };
    other::time::DeltaTime dt;

    bool scene_start = false;

    bool load_finish = false;
    std::mutex loader_mtx;
    other::Scope<std::thread> loader_thread;

    void RunTest();
};

/**
 * Debug Mode (50,000 cubes)
 *  FPS = 10 :(
 *
 * Release Mode (50,000 cubes)
 *  FPS = 60 :)
 **/
TEST_F(StressTest , lots_o_cubes) {
  auto renderer = TEST_ENGINE_ENV()->GetDefaultSceneRenderer(num_cubes);

  float ts = 0.f;
  dt.Start();

  test_running = true;

  do {
    if (scene_start && !timer.Running()) {
      timer.Start();
    }
    ts = dt.Get();

    EmptyEventLoop();
    
    if (load_finish) {
      if (loader_thread != nullptr) {
        loader_thread->join();
      }
      loader_thread = nullptr; 
    } else if (loader_thread == nullptr) {
      loader_thread = other::NewScope<std::thread>(std::bind_front(&StressTest::LoadCubes , this));
    }

    other::Renderer::GetWindow()->Clear();
    if (scene_start) {
      other::DefaultUpdateCamera(camera);

      scene->EarlyUpdate(ts);
      scene->Update(ts);
      scene->LateUpdate(ts);

      renderer->SubmitCamera(camera);
      scene->Render(renderer);  
      renderer->EndScene();

      const auto& frames = renderer->GetRender(); 
      const auto& vp = frames.at(other::kDefaultTargetHash);
      other::Renderer::DrawFramebufferToWindow(vp);
    }

    other::UI::BeginFrame();
    if (scene_start) {
      ImGui::SetNextWindowSize({ 250 , 100 });
      ImGui::Begin("Data");
      ImGui::Text("Time Remaining = %llus" , timer.GetRemainingTime() / 1000000);
      ImGui::Text("num cubes = %d" , num_cubes);
      ImGui::Text("dt = %.1fms" , ts);
      ImGui::Text("FPS = %.1f" , 1.f / (ts / 1000.f));
      ImGui::End();
    } else if (load_finish) {
      ImGui::Begin("Test Menu");
      ImGui::Text("Loaded In %.2fs [%lluus]" , (float)load_time_us / 1000000 , load_time_us);
      ImGui::Separator();
      ImGui::Separator();
      if (ImGui::Button("Begin Test")) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        scene->Start(other::EngineMode::RUNTIME);
        scene->EarlyUpdate(ts);
        scene->Update(ts);
        scene->LateUpdate(ts);
        scene_start = true; 
      }
      ImGui::End();
    } else if (!load_finish && loader_thread != nullptr) {
      ImGui::Begin("Loading...");
      ImGui::Text("%.2f%%" , (float)scene->Registry().view<other::Tag>().size() / num_cubes);
      ImGui::End();
    }

    other::UI::EndFrame();
    other::Renderer::GetWindow()->SwapBuffers();

    GLenum err = glGetError();
    ASSERT_EQ(err , GL_NO_ERROR);
    
    if (scene_start && timer.Running()) {
      if (!timer.Tick()) {
        test_running = false;
      }
    }
  } while (test_running);
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
  
  scene->Initialize();

  auto win_size = other::Renderer::WindowSize();
  camera = NewRef<other::PerspectiveCamera>(glm::ivec2{ win_size.x , win_size.y });
  camera->SetPosition({ 0.f , 0.f , 3.f });
}

void StressTest::TearDown() {
  scene->Stop();
  scene->Shutdown();
  scene = nullptr;
}
    
void StressTest::LoadCubes() {
  thread_local other::RandomGenerator<int32_t> generator(-1 * max_coord , max_coord);

  other::time::Stopwatch stopwatch;
  stopwatch.Start();
  {
    other::Entity* ent = nullptr;
    for (uint32_t i = 0; i < num_cubes; ++i) {
      {
        std::unique_lock l(loader_mtx);
        ent = scene->CreateEntity(other::fmtstr("cube {}" , i));
      }
      auto& mesh = ent->AddComponent<StaticMesh>();
      auto& transform = ent->GetComponent<Transform>();

      mesh.handle = cube_handle;
      mesh.material = {};
      transform.position = {
        generator.Next() , 
        generator.Next() , 
        generator.Next()
      };
    }
  }
  stopwatch.Stop();
    
  std::unique_lock l(loader_mtx);
  load_time_us = stopwatch.GetDuration();
  load_finish = true;
}
void StressTest::RunTest() {}
