/**
 * \file other_engine.hpp
 **/
#ifndef OTHER_ENGINE_HPP
#define OTHER_ENGINE_HPP

#include "engine/engine.hpp"

// foward headers for the client
#include "core/buffer.hpp"
#include "core/config.hpp"
#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "engine/engine.hpp"

#include "application/app.hpp"
#include "application/app_state.hpp"
#include "asset/serializers/scene_serializer.hpp"
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

#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

#include "memory/arena_allocator.hpp"
#include "serialization/serializer.hpp"

namespace other {

#ifndef OTHERENGINE_DLL
  /// implemented by user
  ExitCode Main(int argc, char* argv[]);
#else   // OTHERENGINE_DLL
  /// implemented by the engine
  CLIENT_SIDE ExitCode Main(int argc, char* argv[]);
#endif  // !OTHERENGINE_DLL

}  // namespace other

#ifndef OTHERENGINE_DLL
  #ifdef OE_CLIENT

    #ifdef OE_WINDOWS
      #define WIN32_LEAN_AND_MEAN
      #include <Windows.h>

      #define MAIN_FUNCTION int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)

      #define MAIN_BODY                                       \
        HINSTANCE other_engine_instance = hInstance;          \
        __try {                                               \
          other::ExitCode exit = other::Main(__argc, __argv); \
          return exit;                                        \
        } __except (EXCEPTION_EXECUTE_HANDLER) {              \
          other::println("SEH Exception caught");             \
          return 1;                                           \
        }

    #else  // OE_WINDOWS
      #define MAIN_FUNCTION int main(int argc, char** argv)
      #define MAIN_BODY return other::Main(argc, argv);
    #endif  // OE_WINDOWS

    #define IMPLEMENT_CLIENT_LOADERS(project_name, driver)                                                   \
      namespace other {                                                                                      \
        static ArenaAllocator<project_name> app_allocator;                                                   \
        static ArenaAllocator<driver> driver_allocator;                                                      \
        CLIENT_SIDE App* NewApp(const CmdLine& cmd_line, const ConfigTable& config) {                        \
          static_assert(std::is_base_of_v<App, project_name>, #project_name " must derive from other::App"); \
          return app_allocator.Allocate(cmd_line, config);                                                   \
        }                                                                                                    \
        CLIENT_SIDE Engine* LoadDriver(const CmdLine& cmd_line) {                                            \
          static_assert(std::is_base_of_v<Engine, driver>, #driver " must derive from other::Engine");       \
          return driver_allocator.Allocate(cmd_line);                                                        \
        }                                                                                                    \
        CLIENT_SIDE void FreeApp(App* a) { app_allocator.Free(a); }                                          \
        CLIENT_SIDE void UnloadDriver(Engine* d) { driver_allocator.Free(d); }                               \
      }  // namespace other

    #define ENTRY_POINT(project_name, driver)        \
      IMPLEMENT_CLIENT_LOADERS(project_name, driver) \
      MAIN_FUNCTION {                                \
        MAIN_BODY                                    \
      }

    #define ENTRY_POINT_DEFAULT(project_name) ENTRY_POINT(project_name, other::Engine)
    #define GET_ENTRY_POINT(_1, _2, NAME, ...) NAME

  #else
    #define IMPLEMENT_CLIENT_LOADERS(project_name, driver) static_assert(false, "IMPLEMENT_CLIENT_LOADERS can only be used in client code");
  #endif  // !OE_CLIENT
#else     // OTHERENGINE_DLL
  #error "DLL-Loader UNIMPLEMENTED"
#endif  // !OTHERENGINE_DLL

#ifndef OTHERENGINE_DLL
  #define OTHER_ENTRY_POINT(...) GET_ENTRY_POINT(__VA_ARGS__, ENTRY_POINT, ENTRY_POINT_DEFAULT)(__VA_ARGS__)
#endif  // !OTHERENGINE_DLL

#endif  // !OTHER_ENGINE_HPP
