/**
 * \file unit_tests/timer_tests.cpp
 **/
#include "oetest.hpp"

#include <thread>

#include "core/time.hpp"

class TimerTests : public other::OtherTest {
  public:
};

using namespace std::chrono_literals;

TEST_F(TimerTests , DISABLED_basic_timer_test) {
  other::time::Timer timer(10s); 
  timer.Start();

  for (uint32_t i = 0; i < 10; ++i) {
    EXPECT_TRUE(timer.Tick());
    std::cout << timer.GetRemainingTime() << std::endl;
    std::this_thread::sleep_for(1s);
  }

  EXPECT_FALSE(timer.Tick());
}

TEST_F(TimerTests , DISABLED_basic_stopwatch_test) {
  other::time::Stopwatch stopwatch;
  stopwatch.Start();

  for (uint32_t i = 0; i < 10; ++i) {
    std::this_thread::sleep_for(1s);
  }

  EXPECT_GE(stopwatch.GetDuration() , std::chrono::seconds(10).count());
}
