/**
 * \file core/state.cpp
 **/
#include "core/state.hpp"

namespace other {

  void State::Attach() {
    OnAttach();
  }

  void State::Step() {
    PROFILE_SECTION("State--Step");
    OnStep();
  }

  void State::Detach() {
    OnDetach();
  }

}  // namespace other