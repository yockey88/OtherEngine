/**
 * \file thread/thread.cpp
 **/
#include "thread/thread.hpp"

#include <functional>

#include "core/logger.hpp"

namespace other {

  void Thread::Launch(const ConfigTable& cfg) {
    config = cfg;
    thread = std::make_unique<std::jthread>(std::bind_front(&Thread::Run, this));
  }

  void Thread::Start() {
    init_condition.notify_one();
  }

  void Thread::Join() {
    thread->request_stop();

    thread->join();
  }

  void Thread::Run(std::stop_token stoken) {
    Logger::Instance()->RegisterThread(thread_name);
    Init();

    {
      std::unique_lock<std::mutex> lock(init_mutex);
      init_condition.wait(lock);
    }

    while (!stoken.stop_requested()) {
      Step();
    }

    Shutdown();
  }

} // namespace other
