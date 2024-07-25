/**
 * \file threading_tests/main.cpp
 **/
#include "core/errors.hpp"
#include "core/logger.hpp"
#include "core/channel.hpp"

#include "application/app.hpp"

#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"
#include "thread/thread.hpp"
#include "thread/app_thread.hpp"

#include "common/test_app.hpp"
#include "common/engine_mock.hpp"

using other::ChannelEndpoint;
using other::Channel;
using other::Message;

using other::Thread;

#pragma pack(push , 1)

struct RenderContextMessage {
  SDL_Window* window = nullptr;
};

#pragma pop(push , 1)

int main(int argc , char* argv[]) {
  using namespace std::chrono_literals;

  try {
    other::CmdLine cmd_line(argc , argv);

    std::string config_path = "C:/Yock/code/OtherEngine/tests/threading_tests/threading_tests.other"; 
    other::IniFileParser parser(config_path);
    auto config = parser.Parse(); 

    other::Logger::Open(config);
    other::Logger::Instance()->RegisterThread("Main-Thread");
     
    Channel<Message> channel;
    
    Scope<ChannelEndpoint<Message>>& ep1 = channel.GetFirstEndpoint();
    Scope<ChannelEndpoint<Message>>& ep2 = channel.GetSecondEndpoint();

    Scope<Thread> app_thread = NewScope<other::AppThread>(config , std::move(ep2));
    app_thread->Run();

    std::this_thread::sleep_for(1s);

    other::MessageMetadata meta{
      .type = other::INITIALIZE_MSG ,
      .size = sizeof(RenderContextMessage) ,
    };

    int32_t num = 10;
    
    {
      RenderContextMessage msg {
      };
      Scope<Message> m = NewScope<Message>(meta , msg);

      OE_DEBUG("Sending Init Message");
      ep1->Send(m);
    }

    meta.type = other::START_MSG;
    num = 20;

    {
      auto ready = ep1->Receive();
      
      RenderContextMessage msg {
      };

      Scope<Message> m = NewScope<Message>(meta , msg);
      
      OE_DEBUG("Sending Start Message");
      ep1->Send(m);
    }

    {
      auto ready = ep1->Receive();
    }
    
    std::this_thread::sleep_for(3s);

    app_thread->Join();

    OE_INFO("Successful exit");

    other::Logger::Shutdown();
    return 0;
  } catch (const other::IniException& e) {
    std::cout << "caught ini error : " << e.what() << "\n";
  } catch (const other::ShaderException& e) {
    std::cout << "caught shader error : " << e.what() << "\n";
  } catch(const std::exception& e) {
    std::cout << "caught std error : " << e.what() << "\n";
  } catch (...) {
    std::cout << "caught exception\n";
  }
  return 1;
}
