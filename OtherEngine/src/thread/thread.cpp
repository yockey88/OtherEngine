/**
 * \file thread/thread.cpp
 **/
#include "thread/thread.hpp"

#include <functional>

#include "core/channel.hpp"
#include "core/logger.hpp"

namespace other {

#pragma push(pack , 1)

  struct ReadyMessage {
    bool ready = false; 
  };

#pragma pop(push)
  
  void Thread::Run() {
    thread = std::make_unique<std::jthread>(std::bind_front(&Thread::Main, this));
  }
  
  void Thread::Join() {
    thread->request_stop();
    thread->join();
  }

  void Thread::Main(std::stop_token st) {
    other::Logger::Instance()->RegisterThread(thread_name);

    OE_DEBUG("App-Thread waiting for initialize");
    auto init_msg = endpoint->Receive();

    Init();

    MessageMetadata meta {
      .size = sizeof(ReadyMessage) ,
    };

    
    {
      ReadyMessage msg {
        .ready = true ,
      };
      auto ready_msg = NewScope<Message>(meta , msg);
      endpoint->Send(ready_msg);
    }
    
    OE_DEBUG("App-Thread waiting for start");
    auto start_msg = endpoint->Receive();
    
    {
      ReadyMessage msg {
        .ready = true ,
      };
      
      auto ready_msg = NewScope<Message>(meta , msg);
      endpoint->Send(ready_msg);
    }

    OE_DEBUG("Running Thread {}" , thread_name);
    while (!st.stop_requested()) {
      Step();
    } 

    Shutdown();

    {
      ReadyMessage msg {
        .ready = true ,
      };

      auto ready_msg = NewScope<Message>(meta , msg);
      endpoint->Send(ready_msg);
    }
  }



} // namespace other
