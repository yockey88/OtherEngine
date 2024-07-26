/**
 * \file thread/app_thread.hpp
 **/
#ifndef OTHER_ENGINE_APP_THREAD_HPP
#define OTHER_ENGINE_APP_THREAD_HPP


#include "thread/thread.hpp"

namespace other {

  class AppThread : public Thread {
    public:
      AppThread(struct ThreadRenderContext& context , const ConfigTable& config) 
        : Thread(context , "App-Thread" , config) {}
      virtual ~AppThread() override {}

    private:
      uint32_t counter = 0;
      
      void Init(Scope<Message>& init_msg) override;
      void Shutdown() override;

      void Step() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_THREAD_HPP
