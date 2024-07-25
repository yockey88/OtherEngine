/**
 * \file thread/thread.hpp
 **/
#ifndef OTHER_ENGINE_THREAD_HPP
#define OTHER_ENGINE_THREAD_HPP

#include <stop_token>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "core/config.hpp"
#include "core/channel.hpp"

namespace other {

  class Thread {
    public:
      Thread(const std::string& name , const ConfigTable& config , Scope<ChannelEndpoint<Message>> endpoint) 
        : config(config) , thread_name(name) , endpoint(std::move(endpoint)) {}
      virtual ~Thread() {}

      void Run();
      void Join();

    protected:
      ConfigTable config;

    private:
      std::string thread_name = "UNKNOWN Thread";

      Scope<std::jthread> thread = nullptr;
      std::mutex init_mutex;
      std::condition_variable init_condition;

      Scope<ChannelEndpoint<Message>> endpoint;

      virtual void Step() = 0;
      
      void Main(std::stop_token st);

      virtual void Init() {}
      virtual void Shutdown() {}
  };

} // namespace other

#endif // !OTHER_ENGINE_THREAD_HPP
