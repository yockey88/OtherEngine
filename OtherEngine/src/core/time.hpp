/**
 * \file core/time.hpp
*/
#ifndef OTHER_ENGINE_TIME_HPP
#define OTHER_ENGINE_TIME_HPP

#include <chrono>
#include <thread>

namespace other {
namespace time {

  typedef std::chrono::steady_clock Clock;
  typedef std::chrono::duration<float> Duration;
  typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

  class Timer {
    TimePoint start;
    TimePoint end;
    Duration remaining_time;
    Duration duration;
    Duration paused_duration;

    bool running = false;
    bool repeat = false;

    public:
      Timer(Duration duration , bool repeat = false) 
        : duration(duration) , repeat(repeat) {}
      ~Timer() {}

      void Start();
      void Restart();
      void Pause();

      bool Tick();
      void Reset();

      inline bool Running() const { return running; }
      inline bool Finished() const { return !running; }
      inline float GetRemainingTime() const { return remaining_time.count(); }
      inline float GetDuration() const { return duration.count(); }
  };

  class Stopwatch {
    TimePoint start;
    TimePoint end;
    Duration duration;
    Duration paused_duration;

    bool running = true;
    bool paused = false;
    
    public:
      Stopwatch() : start(Clock::now()) {}
      ~Stopwatch() {}

      void Start();
      void Pause();
      void Tick();
      void Stop();
      void Reset();

      inline float GetDuration() { return duration.count(); }
  };

  class DeltaTime {
    Duration duration;
    TimePoint last_time_point;

    public:
      DeltaTime() {}
      ~DeltaTime() {}

      inline void Start() { last_time_point = Clock::now(); }

      inline float Get() {
        TimePoint current_time_point = Clock::now();
        duration = current_time_point - last_time_point;
        last_time_point = current_time_point;
        return duration.count();
      }
  };

  template <uint32_t fps>
  class FrameRateEnforcer {
    std::chrono::duration<double , std::ratio<1 , fps>> frame_duration;
    std::chrono::time_point<std::chrono::steady_clock , decltype(frame_duration)> time_point;

    public:
      FrameRateEnforcer() { time_point = std::chrono::steady_clock::now(); }

      inline float TimeStep() const { return (float)(1.f / fps); }

      inline void Enforce() {
        time_point += frame_duration;
        std::this_thread::sleep_until(time_point);
      }
  };

} // namespace time
} // namespace other

#endif // !OTHER_ENGINE_TIME_HPP
