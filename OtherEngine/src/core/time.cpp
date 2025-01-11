/**
 * \file core\time.cpp
 */
#include "core\time.hpp"

namespace other {
  namespace time {

    void Timer::Start() {
      if (!running) {
        start = SteadyClock::now();
        remaining_time = duration;
        running = true;
      }
    }

    void Timer::Restart() {
      start = SteadyClock::now();
      remaining_time = duration;
      running = true;
    }

    void Timer::Pause() {
      if (running) {
        end = SteadyClock::now();
        paused_duration = std::chrono::duration_cast<Duration>(end - start);
        running = false;
      }
    }

    bool Timer::Tick() {
      if (!running) {
        return false;
      }

      end = SteadyClock::now();
      auto passed = std::chrono::duration_cast<Duration>(end - start);
      if (passed >= duration) {
        remaining_time = Duration(0);
        running = false;
        return running;
      }

      remaining_time = std::chrono::duration_cast<Duration>(duration - passed);
      if (remaining_time == Duration(0)) {
        running = false;
      }

      return running;
    }

    void Timer::Reset() {
      start = SteadyClock::now();
      remaining_time = duration;
      paused_duration = Duration(0);
      running = true;
    }

    bool Timer::Running() const {
      return running;
    }

    bool Timer::Finished() const {
      return !running;
    }

    uint64_t Timer::GetRemainingTime() const {
      return remaining_time.count();
    }

    uint64_t Timer::GetDuration() const {
      return duration.count();
    }

    void Stopwatch::Start() {
      if (!running) {
        start = SteadyClock::now();
        running = true;
      }

      if (paused) {
        TimePoint unpaused_time = SteadyClock::now();
        paused_duration = std::chrono::duration_cast<Duration>(unpaused_time - end);
        paused = false;
      }
    }

    void Stopwatch::Pause() {
      if (running) {
        end = SteadyClock::now();
        duration = std::chrono::duration_cast<Duration>(end - start);
        running = false;
        paused = true;
      }
    }

    void Stopwatch::Tick() {
      if (running && !paused) {
        duration = std::chrono::duration_cast<Duration>(SteadyClock::now() - start);
      }
    }

    void Stopwatch::Stop() {
      if (running) {
        end = SteadyClock::now();
        duration = std::chrono::duration_cast<Duration>(end - start);
        running = false;
        paused = false;
      }
    }

    void Stopwatch::Reset() {
      if (running && !paused) {
        end = SteadyClock::now();
        duration = std::chrono::duration_cast<Duration>(end - start);
      } else {
        duration = Duration(0);
        paused_duration = Duration(0);
      }
    }

    uint64_t Stopwatch::GetDuration() {
      return duration.count();
    }

  }  // namespace time
}  // namespace other
