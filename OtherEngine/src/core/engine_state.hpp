
/**
 * \file core/engine_state.cpp
 **/
#ifndef OTHER_ENGINE_ENGINE_STATE_HPP
#define OTHER_ENGINE_ENGINE_STATE_HPP

#include "core/defines.hpp"

namespace other {

  class EngineState {
    public:
      static Opt<ExitCode> exit_code;
  };

} // namespace other

#endif // !OTHER_ENGINE_ENGINE_STATE_HPP
