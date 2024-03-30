/**
 * \file other_engine.hpp
 **/
#ifndef OTHER_ENGINE_HPP
#define OTHER_ENGINE_HPP

#include <filesystem>

#include "core/engine.hpp"
#include "parsing/cmd_line_parser.hpp"

namespace other {

  /// implemented by user
  extern Scope<App> NewApp(Engine* engine , const ConfigTable& config);

  class OE {
    public:
      static ExitCode Main(int argc , char* argv[]);

    private:
      static CmdLine cmd_line;
      static ConfigTable current_config;

      static std::filesystem::path config_path;

      Engine engine;

      static void Help();
      static void Version();

      static std::string FindConfigFile();
      static bool LoadConfig();

      static void CoreInit();

      void Launch();
      ExitCode Run();
      void Shutdown();
      static void HandleExit(ExitCode code);

      static void CoreShutdown();
  };

} // namespace other

#define OE_APPLICATION(project_name) \
  namespace other { \
    Scope<App> NewApp(Engine* engine , const ConfigTable& config) { \
      static_assert(std::is_base_of_v<App , project_name> , #project_name " must derive from other::App"); \
      return NewScope<project_name>(engine); \
    } \
  } \
  int main(int argc , char* argv[]) { \
    try { \
      other::ExitCode exit = other::OE::Main(argc , argv); \
      if (exit != other::ExitCode::SUCCESS) { \
        return 1; \
      } \
    } catch (std::exception& e) { \
      std::cerr << e.what() << std::endl; \
      return 1; \
    } \
    return 0; \
  }

#endif // !OTHER_ENGINE_HPP
