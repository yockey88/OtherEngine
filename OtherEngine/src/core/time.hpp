/**
 * \file core/time.hpp
*/
#ifndef OTHER_ENGINE_TIME_HPP
#define OTHER_ENGINE_TIME_HPP

#include <chrono>
#include <thread>

namespace other {
namespace time {

  using Clock = std::chrono::steady_clock;
  using Duration = std::chrono::duration<uint64_t , std::micro>;
  using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

  class Timer {
    TimePoint start;
    TimePoint end;
    Duration remaining_time;
    Duration duration;
    Duration paused_duration;

    bool running = false;
    bool repeat = false;

    public:
      template <typename T>
      Timer(T duration , bool repeat = false) 
        : duration(std::chrono::duration_cast<Duration>(duration)) , repeat(repeat) {}
      ~Timer() {}

      void Start();
      void Restart();
      void Pause();

      bool Tick();
      void Reset();

      bool Running() const;
      bool Finished() const;
      uint64_t GetRemainingTime() const;
      uint64_t GetDuration() const;
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

      uint64_t GetDuration();
  };

  class DeltaTime {
    Duration duration;
    TimePoint last_time_point;

    public:
      DeltaTime() {}
      ~DeltaTime() {}

      inline void Start() { 
        last_time_point = Clock::now(); 
      }

      inline float Get() {
        TimePoint current_time_point = Clock::now();
        duration = std::chrono::duration_cast<Duration>(current_time_point - last_time_point);
        last_time_point = current_time_point;
        return (float)duration.count() / 1000;
      }

      template <typename T>
      inline float GetAs() {
        TimePoint current_time_point = Clock::now();
        duration = std::chrono::duration_cast<T>(current_time_point - last_time_point);
        last_time_point = current_time_point;
        return (float)duration.count();
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
