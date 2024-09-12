#include <cstddef>
#include <string_view>

#include <pybind11/pybind11.h>

#include "core/defines.hpp"
#include "editor/editor_asset_handler.hpp"
#include "event/key_events.hpp"
#include "event/window_events.hpp"
#include "other_engine.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "scene/environment.hpp"

using namespace other;
namespace py = pybind11;

namespace {

class EnvironmentApp : public App {
  public:
    EnvironmentApp(const CmdLine& cmdline, const ConfigTable& config) 
      : App(cmdline, config) {}
    virtual ~EnvironmentApp() override {}

      // virtual void OnLoad();
      virtual void OnAttach() override; 

      virtual void OnEvent(Event* event) override;

      // virtual void EarlyUpdate(float dt) override;
      virtual void Update(float dt) override;
      // virtual void LateUpdate(float dt) override;

      virtual void Render() override;
      // virtual void RenderUI() override;

      virtual void OnDetach() override;
      // virtual void OnUnload() override;

      // virtual void OnSceneLoad(const SceneMetadata* path) override;
      // virtual void OnSceneUnload() override;

      /// will only ever be called if editor is active because otherwise the scripts
      ///   wont be reloaded
      // virtual void OnScriptReload() override;

      virtual Ref<AssetHandler> CreateAssetHandler() override;
};

} // anonymous namespace

namespace {

class PyEnv {
  public:
    static Opt<Path> env_cfg_path;
    
    static Scope<App> application;
    static CmdLine cmd_line;
    static Opt<ConfigTable> config;

    //// exit condition
    static Opt<ExitCode> exit_code;
};

bool ValidateConfigPath();
bool LoadConfig();
void LoadSystems();
void ShutdownSystems();

template <typename Fn, typename... Args>
ExitCode RunEnvironmentTask(Fn f, Args&&... args) {
  
}

ExitCode OpenScene(const std::string_view scene) {
  println("Loading environment config...");
  auto cfg = LoadConfig();
  if (!cfg) {
    return ExitCode::FAILURE;
  }
  
  try {
    println("Loading other environment...");
    LoadSystems();
    
    // // AppState::Scenes()->LoadScene(scene);
    // // AppState::Scenes()->SetAsActive(scene);

    // println("running...");
    // do {
    //   PyEnv::application->Run();
    // } while (!PyEnv::exit_code.has_value());

    println("shutting down environment...");
    ShutdownSystems();
  } catch (std::exception& e) {
    println("Failed to load environment : {}", e.what());
    return ExitCode::FAILURE;
  } catch (...) {
    println("Failed to environment : unknown error");
    return ExitCode::FAILURE;
  }

  // println("exiting with code {}", PyEnv::exit_code.value());
  return ExitCode::SUCCESS; //PyEnv::exit_code.value();
}

struct LogWrapper {
  void Trace(const std::string& msg);
  void Debug(const std::string& msg);
  void Info(const std::string& msg);
  void Warn(const std::string& msg);
  void Error(const std::string& msg);
  void Critical(const std::string& msg);
};

} // anonymous namespace

PYBIND11_MODULE(engine, m) {
  py::class_<LogWrapper>(m, "Logger")
    .def("trace", &LogWrapper::Trace)
    .def("debug", &LogWrapper::Debug)
    .def("info", &LogWrapper::Info)
    .def("warn", &LogWrapper::Warn)
    .def("error", &LogWrapper::Error)
    .def("critical", &LogWrapper::Critical);

  py::class_<PyEnv>(m, "Native")
    .def("set_cfg_path", [&](const std::string& str) { PyEnv::env_cfg_path = str; })
    .def("get_cfg_path", []() { return PyEnv::env_cfg_path.value_or("<empty>"); });

  py::enum_<ExitCode>(m, "ExitCode")
    .value("SUCCESS", ExitCode::SUCCESS)
    .value("FAILURE", ExitCode::FAILURE)
    .value("NO_EXIT", ExitCode::NO_EXIT);

  // py::class_<Arg>(m, "Arg")
  //   .def(py::init<>())
  //   .def_readwrite("hash", &Arg::hash)
  //   .def_readwrite("flag", &Arg::flag)
  //   .def_readwrite("args", &Arg::args);

  m.def("open_scene", &OpenScene);
  m.def("fnv_hash" , &FNV);
}

namespace {
  
void EnvironmentApp::OnAttach() {
}

void EnvironmentApp::OnEvent(Event* event) {
  EventHandler handler(event);
  handler.Handle<WindowClosed>([](WindowClosed& e) -> bool {
    PyEnv::exit_code = ExitCode::SUCCESS;
    return true;
  });

  handler.Handle<KeyPressed>([](KeyPressed& e) -> bool {
    if (e.Key() == Keyboard::Key::OE_ESCAPE) {
      PyEnv::exit_code = ExitCode::SUCCESS;
      return true;
    }
    return false;
  });
}

void EnvironmentApp::Update(float dt) {
}

void EnvironmentApp::Render() {
}

void EnvironmentApp::OnDetach() {
}

Ref<AssetHandler> EnvironmentApp::CreateAssetHandler() {
  return NewRef<EditorAssetHandler>();
}

Opt<Path> PyEnv::env_cfg_path = std::nullopt;

Scope<App> PyEnv::application = nullptr;
CmdLine PyEnv::cmd_line;
Opt<ConfigTable> PyEnv::config;

Opt<ExitCode> PyEnv::exit_code = std::nullopt;

bool ValidateConfigPath() {
  if (PyEnv::env_cfg_path.has_value()) {
    return Filesystem::PathExists(PyEnv::env_cfg_path.value());
  }
  return false;
}

bool LoadConfig() {
  try {
    if (!ValidateConfigPath()) {
      println(" > Environment config path is not valid");
      return false;
    }

    IniFileParser parser(PyEnv::env_cfg_path->string());
    try {
      PyEnv::config = parser.Parse();
    } catch (IniException& e) {
      println(" > Failed to parse INI file : {}", e.what());
      return false;
    }
    println(" > Environment config loaded");
  } catch (std::exception& e) {
    println("> Failed to load environment config : {}", e.what());
    return false;
  } catch (...) {
    println(" > Failed to load environment config : unknown error");
    return false;
  }

  return true;
}

void LoadSystems() {
  other::Logger::Open(*PyEnv::config);
  other::Logger::Instance()->RegisterThread("Other Environment");
  


  // PyEnv::application = NewScope<EnvironmentApp>(PyEnv::cmd_line, *PyEnv::config);
  // PyEnv::application->Load();
  // AppState::Initialize(PyEnv::application.get() , PyEnv::application->layer_stack , 
  //                       PyEnv::application->scene_manager , PyEnv::application->asset_handler , 
  //                       PyEnv::application->project_metadata);
  
  IO::Initialize();
  EventQueue::Initialize(*PyEnv::config);

  Renderer::Initialize(*PyEnv::config);
  CHECKGL();

  UI::Initialize(*PyEnv::config , Renderer::GetWindow());
  ScriptEngine::Initialize(*PyEnv::config);

  PhysicsEngine::Initialize(*PyEnv::config); 
}

void ShutdownSystems() {
  PhysicsEngine::Shutdown();

  ScriptEngine::Shutdown();

  UI::Shutdown();
  Renderer::Shutdown();
  EventQueue::Shutdown();
  IO::Shutdown();

  // PyEnv::application->Unload();
  other::Logger::Shutdown();
}

void LogWrapper::Trace(const std::string& msg) {
  OE_TRACE(" py > {}", msg);
}
void LogWrapper::Debug(const std::string& msg) {
  OE_DEBUG(" py > {}", msg);
}
void LogWrapper::Info(const std::string& msg) {
  OE_INFO(" py > {}", msg);
}
void LogWrapper::Warn(const std::string& msg) {
  OE_WARN(" py > {}", msg);
}
void LogWrapper::Error(const std::string& msg) {
  OE_ERROR(" py > {}", msg);
}
void LogWrapper::Critical(const std::string& msg) {
  OE_CRITICAL(" py > {}", msg);
}

} // anonymous namespace