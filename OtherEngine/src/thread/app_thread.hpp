/**
 * \file thread/app_thread.hpp
 **/
#ifndef OTHER_ENGINE_APP_THREAD_HPP
#define OTHER_ENGINE_APP_THREAD_HPP


#include "thread/thread.hpp"

namespace other {

  class AppThread : public Thread {
    public:
      AppThread(const ConfigTable& config , Scope<ChannelEndpoint<Message>> endpoint) 
        : Thread("App-Thread" , config , std::move(endpoint)) {}
      ~AppThread() {}

    private:
      uint32_t counter = 0;
      
      void Init() override;
      void Shutdown() override;

      void Step() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_THREAD_HPP
