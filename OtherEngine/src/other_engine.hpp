/**
 * \file other_engine.hpp
 **/
#ifndef OTHER_ENGINE_HPP
#define OTHER_ENGINE_HPP

#ifdef _WIN32
#ifndef OE_MODULE
#define OE_API extern "C" __declspec(dllexport)
#else
#define OE_API extern "C" __declspec(dllimport)
#endif  // !OE_MODULE
#else
#ifndef OE_MODULE
#define OE_API __attribute__((visibility("default")))
#else
#define OE_API
#endif  // !OE_MODULE
#endif  // _WIN32

#include "engine/engine.hpp"

// foward headers for the client
#include "core/buffer.hpp"
#include "core/config.hpp"
#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "application/app.hpp"
#include "application/app_state.hpp"
#include "event/app_events.hpp"
#include "event/core_events.hpp"
#include "event/event.hpp"
#include "event/event_handler.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "event/mouse_events.hpp"
#include "event/scene_events.hpp"
#include "event/ui_events.hpp"
#include "event/window_events.hpp"
#include "input/io.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"

#include "scene/scene.hpp"
#include "scene/scene_serializer.hpp"

#include "physics/phyics_engine.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

#include "engine/engine.hpp"
#include "serialization/serializer.hpp"

namespace other {

#ifndef OTHERENGINE_DLL
  /// implemented by user
  ExitCode Main(int argc, char* argv[]);
#endif  // !OTHERENGINE_DLL

}  // namespace other

#ifndef OTHERENGINE_DLL
/// defines the entry poing for clients
/// TODO:
///   - add os signal handlers
///   - remove this legacy macro
#define OE_APPLICATION(project_name)                                                                     \
  namespace other {                                                                                      \
    App* NewApp(const CmdLine& cmd_line, const ConfigTable& config) {                                    \
      static_assert(std::is_base_of_v<App, project_name>, #project_name " must derive from other::App"); \
      return new project_name(cmd_line, config);                                                         \
    }                                                                                                    \
  }                                                                                                      \
  int main(int argc, char* argv[]) {                                                                     \
    using namespace other;                                                                               \
    ExitCode exit = Main(argc, argv);                                                                    \
    return exit;                                                                                         \
  }

#ifdef OE_WINDOWS
#define OTHER_ENTRY_POINT(project_name)                                                                  \
  namespace other {                                                                                      \
    App* NewApp(const CmdLine& cmd_line, const ConfigTable& config) {                                    \
      static_assert(std::is_base_of_v<App, project_name>, #project_name " must derive from other::App"); \
      return new project_name(cmd_line, config);                                                         \
    }                                                                                                    \
  }                                                                                                      \
  static HINSTANCE other_engine_instance = nullptr;                                                      \
  int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {      \
    other_engine_instance = hInstance;                                                                   \
    __try {                                                                                              \
      using namespace other;                                                                             \
      ExitCode exit = Main(__argc, __argv);                                                              \
      return exit;                                                                                       \
    } __except (EXCEPTION_EXECUTE_HANDLER) {                                                             \
      other::println("SEH Exception caught");                                                            \
      return 1;                                                                                          \
    }                                                                                                    \
  }
#else
#define OTHER_ENTRY_POINT(project_name)                                                                  \
  namespace other {                                                                                      \
    App* NewApp(const CmdLine& cmd_line, const ConfigTable& config) {                                    \
      static_assert(std::is_base_of_v<App, project_name>, #project_name " must derive from other::App"); \
      return new project_name(cmd_line, config);                                                         \
    }                                                                                                    \
  }                                                                                                      \
  int main(int argc, char** argv) {                                                                      \
    using namespace other;                                                                               \
    ExitCode exit = Main(argc, argv);                                                                    \
  }
#endif

#endif  // !OTHERENGINE_DLL

#endif  // !OTHER_ENGINE_HPP
