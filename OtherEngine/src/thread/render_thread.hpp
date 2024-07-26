/**
 * \file thread/render_thread.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_THREAD_HPP
#define OTHER_ENGINE_RENDER_THREAD_HPP

#include "core/config.hpp"
#include "thread/thread.hpp"

namespace other {
  
  class RenderThread : public Thread {
    public:
      RenderThread(const ThreadRenderContext& context , const ConfigTable& config)
        : Thread(context , "Render-Thread" , config) {}
      virtual ~RenderThread() override {}

    private:
      void Init(Scope<Message>& init_msg) override;
      void Shutdown() override;

      void Step() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_RENDER_THREAD_HPP
