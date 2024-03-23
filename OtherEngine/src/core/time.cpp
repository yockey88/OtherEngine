/**
 * \file core\time.cpp
*/
#include "core\time.hpp"

namespace other {
namespace time {

  void Timer::Start() {
    if (!running) {
      start = Clock::now();
      remaining_time = duration;
      running = true;
    }
  }
  
  void Timer::Restart() {
    start = Clock::now();
    remaining_time = duration;
    running = true;
  }

  void Timer::Pause() {
    if (running) {
      end = Clock::now();
      paused_duration = end - start;
      running = false;
    }
  }

  bool Timer::Tick() {
    if (running) {
      end = Clock::now();
      remaining_time = duration - (end - start);
      if (remaining_time.count() <= 0)
          running = false;
    }
    return running;
  }

  void Timer::Reset() {
    start = Clock::now();
    remaining_time = duration;
    paused_duration = Duration(0);
    running = true;
  }

  void Stopwatch::Start() { 
    if (!running) {
      start = Clock::now();
      running = true;
    }

    if (paused) {
      TimePoint unpaused_time = Clock::now();
      paused_duration = unpaused_time - end;
      paused = false;
    }
  }

  void Stopwatch::Pause() { 
    if (running) {
      end = Clock::now();
      duration = end - start;
      running = false;
      paused = true;
    }
  }
  
  void Stopwatch::Tick() {
    if (running && !paused) {
      duration = Clock::now() - start;
    }
  }
  
  void Stopwatch::Stop() {
    if (running) {
      end = Clock::now();
      duration = end - start;
      running = false;
      paused = false;
    }
  }

  void Stopwatch::Reset() {
    if (running && !paused) {
      end = Clock::now();
      duration = end - start;
    } else {
      duration = Duration(0);
      paused_duration = Duration(0);
    }
  }

} // namespace time
} // namespace other