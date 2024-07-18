using System;
using System.Runtime.CompilerServices;

namespace Other {

  enum State : int {
    PRESSED = 0,
    BLOCKED ,
    HELD ,
    RELEASED ,
  }  

  public struct InputState {
    State current_state , previous_state;
    int frame_count;
  }

}
