/**
 * \file environment/environment.hpp
 **/
#ifndef OTHER_ENGINE_ENVIRONMENT_HPP
#define OTHER_ENGINE_ENVIRONMENT_HPP

#include "environment/env_defines.hpp"
#include "environment/memory.hpp"
#include "environment/terminal.hpp"

#include "event/key_events.hpp"

namespace other {

  struct Environment {
    Memory memory;
    Terminal terminal;

    bool terminal_open = false;

    static void Initialize();
    static void Shutdown();

    static void RenderTerminal();
    static Environment& Get();
    static void PushTerminalMessage(const TerminalMessage& message);
    static bool ReadTerminalInput();

   private:
    static Environment* instance;

    Environment();
    ~Environment();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ENVIRONMENT_HPP
