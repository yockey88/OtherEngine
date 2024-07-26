/**
 * \file thread/thread.cpp
 **/
#include "thread/thread.hpp"

#include <SDL_video.h>
#include <functional>
#include <mutex>

#include "core/channel.hpp"
#include "core/logger.hpp"

#include "thread/message.hpp"

namespace other {
  
  void Thread::Run() {
    thread = std::make_unique<std::jthread>(std::bind_front(&Thread::Main, this));
  }
  
  void Thread::Join() {
    thread->request_stop();
    thread->join();
  }

  void Thread::RegisterEndpoint(UUID id , Scope<ChannelEndpoint<Message>>& endpoint) {
    if (endpoint == nullptr) {
      OE_ERROR("Registering null endpoint!");
      return;
    }
    
    if (endpoints.find(id) != endpoints.end()) {
      OE_DEBUG("Channel ID already exists on this thread {}" , id);
      return;
    }

    endpoints[id] = std::move(endpoint);
  }

  UUID Thread::GetID() const {
    return id;
  }
      
  Scope<ChannelEndpoint<Message>>& Thread::GetCoreChannel() {
    return endpoints[kCoreEndpointId];
  }
      
  std::vector<Scope<Message>> Thread::PollChannels() {
    return {}; 
  }

  bool Thread::ProcessInitMessage(Scope<Message>& init_msg) {
    bool ready = MsgEquals<InitializeMessage>(init_msg , INITIALIZE_MSG); 
    
    if (context.mtx != nullptr) {
      /// this function is not thread safe, most lock
      std::unique_lock<std::mutex> lock(*context.mtx);
      SDL_GL_MakeCurrent(context.window , context.context);
    } else {
      OE_DEBUG("Receieved null mutex!");
      ready = false;
    }

    Init(init_msg);

    if (ready) {
      process_msg = THREAD_MSG_FN(Thread::ProcessStartMessage);
    }

    return ready;
  }

  bool Thread::ProcessStartMessage(Scope<Message>& start_msg) {
    bool ready = MsgEquals<StartMessage>(start_msg , START_MSG);

    if (ready) {
      process_msg = THREAD_MSG_FN(Thread::ProcessMessage);
      main_loop = true;
    }

    return ready;
  }

  bool Thread::ProcessMessage(Scope<Message>& msg) {
    return false;
  }
      
  void Thread::SendReadyMessage(bool ready) {
    MessageMetadata meta {
      .type = READY_MSG ,
      .size = sizeof(ReadyMessage) ,
    };
    
    {
      ReadyMessage msg {
        .ready = ready ,
      };
      auto ready_msg = NewScope<Message>(meta , msg);
      GetCoreChannel()->Send(ready_msg);
    }
  }

  void Thread::Main(std::stop_token st) {
    using namespace std::chrono_literals;
    other::Logger::Instance()->RegisterThread(thread_name);

    OE_ASSERT(process_msg != nullptr , "Thread never initialized message processing function!");

    auto& core_channel = GetCoreChannel();
    OE_ASSERT(core_channel != nullptr , "Thread has null core channel!");

    while (!main_loop) {
      bool ready = false;
      auto msg = core_channel->Receive(0ms);
      ready = process_msg(msg); 
      SendReadyMessage(ready);

      if (!ready) {
        SDL_GL_MakeCurrent(nullptr , nullptr);
        return;
      }
    }

    OE_DEBUG("Running Thread {}" , thread_name);
    while (main_loop && !st.stop_requested()) {
      try {
        Step();
      } catch (const std::exception& e) {
        OE_ERROR("{} caught exception : {}" , thread_name , e.what());
      } catch (...) {
        OE_ERROR("{} caught unknown exception" , thread_name);
      }
    } 

    Shutdown();
    SendReadyMessage(true);
  }

} // namespace other
