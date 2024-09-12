/**
 * \file other_engine.hpp
 **/
#ifndef OTHER_ENGINE_HPP
#define OTHER_ENGINE_HPP

#include "core/engine.hpp"

// foward headers for the client
#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/config.hpp"
#include "core/filesystem.hpp"
#include "core/engine.hpp"
#include "core/serializer.hpp"
#include "core/buffer.hpp"
#include "core/engine.hpp"
#include "input/io.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"
#include "application/app.hpp"
#include "application/app_state.hpp"

#include "event/event.hpp"
#include "event/core_events.hpp"
#include "event/app_events.hpp"
#include "event/key_events.hpp"
#include "event/mouse_events.hpp"
#include "event/window_events.hpp"
#include "event/ui_events.hpp"
#include "event/scene_events.hpp"
#include "event/event_handler.hpp"
#include "event/event_queue.hpp"

#include "scene/scene.hpp"
#include "scene/scene_serializer.hpp"

#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"
#include "physics/phyics_engine.hpp"

namespace other {

  /// implemented by user
  ExitCode Main(int argc , char* argv[]);

} // namespace other

/// defines the entry poing for clients 
/// TODO: make this platform specific and add os signal handlers 
#define OE_APPLICATION(project_name) \
  namespace other { \
    Scope<App> NewApp(const CmdLine& cmd_line, const ConfigTable& config) { \
      static_assert(std::is_base_of_v<App , project_name> , #project_name " must derive from other::App"); \
      return NewScope<project_name>(cmd_line, config); \
    } \
  } \
  int main(int argc , char* argv[]) { \
    using namespace other;\
    ExitCode exit = Main(argc , argv); \
    return exit; \
  }

#endif // !OTHER_ENGINE_HPP
