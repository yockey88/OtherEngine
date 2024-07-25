/**
 * \file thread/render_thread.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_THREAD_HPP
#define OTHER_ENGINE_RENDER_THREAD_HPP

#include "core/channel.hpp"
#include "core/config.hpp"
#include "thread/thread.hpp"

namespace other {
  
  class RenderThread : public Thread {
    public:
      RenderThread(const ConfigTable& config , Scope<ChannelEndpoint<Message>> endpoint)
        : Thread("Render-Thread" , config , std::move(endpoint)) {}
      virtual ~RenderThread() override {}

    private:
      void Init() override;
      void Shutdown() override;

      void Step() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_RENDER_THREAD_HPP
