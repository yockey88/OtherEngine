/**
 * \file core/state.cpp
 **/
#include "core/state.hpp"

namespace other {

  void State::Step() {
    OnStep();
  }

  void StateMachine::Step() {
    OnStep();
  }

}  // namespace other