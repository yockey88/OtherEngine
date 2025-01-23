/**
 * \file sandbox/main.cpp
 **/
#include "other_engine.hpp"

class Game2025 : public other::App {
 public:
  Game2025(const other::CmdLine& cmd_line, const other::ConfigTable& config)
      : other::App(cmd_line, config) {}
  virtual ~Game2025() override {}
};

OTHER_ENTRY_POINT(Game2025);
