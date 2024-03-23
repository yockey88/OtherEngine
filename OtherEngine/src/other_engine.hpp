/**
 * \file other_engine.hpp
 **/
#ifndef OTHER_ENGINE_HPP
#define OTHER_ENGINE_HPP

#include <filesystem>

#include "core/engine.hpp"
#include "parsing/cmd_line_parser.hpp"

namespace other {

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

      static void CoreInit();
      static bool LoadConfig();

      void Launch();
      ExitCode Run();
      static void HandleExit(ExitCode code);

      static void CoreShutdown();
  };

} // namespace other

#endif // !OTHER_ENGINE_HPP
