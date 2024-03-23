/**
 * \file thread/thread.hpp
 **/
#ifndef OTHER_ENGINE_THREAD_HPP
#define OTHER_ENGINE_THREAD_HPP

#include <thread>
#include <mutex>
#include <condition_variable>

#include "core/config.hpp"

namespace other {

  class Thread {
    public:
      Thread(const std::string& name) 
        : thread_name(name) {}
      ~Thread() {}

      void Launch(const ConfigTable& config);
      void Start();
      void Join();

    protected:
      ConfigTable config;

    private:
      std::string thread_name = "UNKNOWN Thread";

      Scope<std::jthread> thread = nullptr;
      std::mutex init_mutex;
      std::condition_variable init_condition;

      void Run(std::stop_token stoken);

      virtual void Init() {}
      virtual void Step() {}
      virtual void Shutdown() {}
  };

} // namespace other

#endif // !OTHER_ENGINE_THREAD_HPP
