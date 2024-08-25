/**
 * \file other_engine.hpp
 **/
#ifndef OTHER_ENGINE_HPP
#define OTHER_ENGINE_HPP

#include "core/engine.hpp"

namespace other {

  /// implemented by user
  extern Scope<App> NewApp(Engine* engine);

  ExitCode Main(int argc , char* argv[]);

  int ProcessExitCode(ExitCode code);

} // namespace other

/// forwarded headers for the client (currently doesnt do anything)
#include "application/app.hpp"

/// defines the entry poing for clients 
/// TODO: make this platform specific and add os signal handlers 
#define OE_APPLICATION(project_name) \
  namespace other { \
    Scope<App> NewApp(Engine* engine) { \
      static_assert(std::is_base_of_v<App , project_name> , #project_name " must derive from other::App"); \
      return NewScope<project_name>(engine); \
    } \
  } \
  int main(int argc , char* argv[]) { \
    other::ExitCode exit = other::Main(argc , argv); \
    return other::ProcessExitCode(exit); \
  }

#endif // !OTHER_ENGINE_HPP
