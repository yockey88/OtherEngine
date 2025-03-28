/**
 * \file forest/forest.cpp
 **/
#include "other_engine.hpp"

class Forest : public other::App {
 public:
  Forest(const other::CmdLine& cmd_line, const other::ConfigTable& config)
      : other::App(cmd_line, config) {}
  virtual ~Forest() override {}
};

OTHER_ENTRY_POINT(Forest);
