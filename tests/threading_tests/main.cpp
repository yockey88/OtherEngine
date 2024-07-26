/**
 * \file threading_tests/main.cpp
 **/
#include "core/errors.hpp"
#include "core/logger.hpp"
#include "core/channel.hpp"
#include "core/config_keys.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"

#include "thread/message.hpp"
#include "thread/channel.hpp"
#include "thread/thread.hpp"
#include "thread/app_thread.hpp"
#include "thread/render_thread.hpp"

#include "common/test_app.hpp"
#include "common/engine_mock.hpp"

#include <SDL.h>
#include <SDL_video.h>

using other::ChannelEndpoint;
using other::Channel;
using other::Message;

using other::Thread;

void EnableSettings(const other::ConfigTable& config) {
  int32_t gl_major = config.GetVal<uint32_t>(other::kRendererSection , other::kMajorValue).value_or(4);
  int32_t gl_minor = config.GetVal<uint32_t>(other::kRendererSection , other::kMinorValue).value_or(6);
  int32_t double_buffer = config.GetVal<bool>(other::kRendererSection , other::kDoubleBufferValue).value_or(true) ? 1 : 0;
  int32_t depth_size = config.GetVal<uint32_t>(other::kRendererSection , other::kDepthSizeValue).value_or(24);
  int32_t stencil_size = config.GetVal<uint32_t>(other::kRendererSection , other::kStencilSizeValue).value_or(8);
  int32_t accelerated_visual = config.GetVal<bool>(other::kRendererSection , other::kAccelVisualValue).value_or(true) ? 1 : 0;
  int32_t multisample_buffers = config.GetVal<uint32_t>(other::kRendererSection , other::kMultisampleBuffersValue).value_or(1);
  int32_t multisample_samples = config.GetVal<uint32_t>(other::kRendererSection , other::kMultisampleSamplesValue).value_or(16);
  int32_t srgb_capable = config.GetVal<bool>(other::kRendererSection , other::kSrgbCapableValue).value_or(true) ? 1 : 0;


  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION , gl_major);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION , gl_minor);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK , SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER , double_buffer);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE , depth_size);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE , stencil_size);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL , accelerated_visual);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS , multisample_buffers);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES , multisample_samples);
  SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE , srgb_capable);
  SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT , 1);

  int32_t vsync = config.GetVal<bool>(other::kRendererSection , other::kVsyncValue).value_or(true) ? 1 : 0;
  SDL_GL_SetSwapInterval(vsync);
}

int main(int argc , char* argv[]) {
  using namespace std::chrono_literals;

  try {
    other::CmdLine cmd_line(argc , argv);

    std::string config_path = "C:/Yock/code/OtherEngine/tests/threading_tests/threading_tests.other"; 
    other::IniFileParser parser(config_path);
    auto config = parser.Parse(); 

    other::Logger::Open(config);
    other::Logger::Instance()->RegisterThread("Main-Thread");

    other::Engine mock_engine = other::Engine::Create(cmd_line , config , config_path);
    {
      Scope<other::App> app = NewScope<TestApp>(&mock_engine);
      mock_engine.LoadApp(app);
    }

    OE_TRACE("Initializing Window");
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
      OE_ERROR("Failed to initialize SDL");
      return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Threading Tests" , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED , 1280 , 720 , SDL_WINDOW_OPENGL); 
    if (window == nullptr) {
      OE_ERROR("Failed to create window!");
      return 1;
    }

    EnableSettings(config);

    SDL_GLContext raw_app_ctx = SDL_GL_CreateContext(window);
    SDL_GLContext raw_render_ctx = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
      SDL_DestroyWindow(window);
      other::Logger::Shutdown();
      return 1;
    } 

    std::mutex ctx_mtx;

    other::ThreadRenderContext app_ctx {
      .window = window ,
      .context = raw_app_ctx ,
      .mtx = &ctx_mtx ,
    };
    
    other::ThreadRenderContext render_ctx {
      .window = window ,
      .context = raw_render_ctx ,
      .mtx = &ctx_mtx ,
    };
    
    Scope<Thread> app_thread = NewScope<other::AppThread>(app_ctx , config);
    Scope<Thread> render_thread = NewScope<other::RenderThread>(render_ctx , config);

    OE_DEBUG("Connecting app thread to core thread");
    Scope<ChannelEndpoint<Message>> app_ep = nullptr;
    {
      Channel<Message> app_core_channel = other::GetChannel<Message>(); 
      app_ep = std::move(app_core_channel.first);
      app_thread->RegisterEndpoint(other::UUID{ 1 } , app_core_channel.second);
    }
    
    OE_DEBUG("Connecting render thread to core thread");
    Scope<ChannelEndpoint<Message>> render_ep = nullptr;
    {
      Channel<Message> render_core_channel = other::GetChannel<Message>(); 
      render_ep = std::move(render_core_channel.first);
      render_thread->RegisterEndpoint(other::UUID{ 1 } , render_core_channel.second);
    }

    OE_DEBUG("Connecting app thread to render thread");
    {
      Channel<Message> app_render_channel = other::GetChannel<Message>();
      app_thread->RegisterEndpoint(FNV("Render-Thread") , app_render_channel.first);
      render_thread->RegisterEndpoint(FNV("App-Thread") , app_render_channel.second);
    }

    app_thread->Run();
    render_thread->Run();

    std::this_thread::sleep_for(1s);

    other::MessageMetadata meta{
      .type = other::INITIALIZE_MSG ,
    };
    
    {
      other::InitializeMessage msg {};
      Scope<Message> m1 = NewScope<Message>(meta , msg);
      Scope<Message> m2 = NewScope<Message>(meta , msg);

      OE_DEBUG("Sending Init Message");
      app_ep->Send(m1);
      render_ep->Send(m2);
    }
    
    std::this_thread::sleep_for(1s);

    {
      auto app_ready = app_ep->Receive(1000ms);
      auto render_ready = render_ep->Receive(1000ms);
      if (other::MsgEquals<other::ReadyMessage>(app_ready , other::READY_MSG) && 
          other::MsgEquals<other::ReadyMessage>(render_ready , other::READY_MSG) &&
          app_ready->Get<other::ReadyMessage>().ready && render_ready->Get<other::ReadyMessage>().ready) {
        other::StartMessage msg{};
      
        meta.type = other::START_MSG;
        Scope<Message> m1 = NewScope<Message>(meta , msg);
        Scope<Message> m2 = NewScope<Message>(meta , msg);
        
        OE_DEBUG("Sending Start Message");
        app_ep->Send(m1);
        render_ep->Send(m2);
      } else {
        OE_ERROR("Failed to initialize threads!"); 
        other::Logger::Shutdown();
        SDL_GL_DeleteContext(app_ctx.context);
        SDL_DestroyWindow(app_ctx.window);
        return 1;
      }
    }
    
    {
      auto app_ready = app_ep->Receive(1000ms);
      auto render_ready = render_ep->Receive(1000ms);
      if (!other::MsgEquals<other::ReadyMessage>(app_ready , other::READY_MSG) ||
          !other::MsgEquals<other::ReadyMessage>(render_ready , other::READY_MSG) ||
          !app_ready->Get<other::ReadyMessage>().ready || !render_ready->Get<other::ReadyMessage>().ready) {
        OE_ERROR("Failed to start threads!"); 
        other::Logger::Shutdown();
        SDL_GL_DeleteContext(app_ctx.context);
        SDL_DestroyWindow(app_ctx.window);
        return 1;
      }
    }

    OE_INFO("Successfully initialized threads!");
    
    std::this_thread::sleep_for(15s);

    app_thread->Join();
    render_thread->Join();

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
