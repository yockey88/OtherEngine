/**
 * \file thread/thread.hpp
 **/
#ifndef OTHER_ENGINE_THREAD_HPP
#define OTHER_ENGINE_THREAD_HPP

#include <stop_token>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <SDL.h>
#include <glad/glad.h>

#include "core/config.hpp"
#include "core/channel.hpp"
#include "core/uuid.hpp"

namespace other {

  struct ThreadRenderContext {
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    std::mutex* mtx = nullptr;
  };

  using ThreadMessageFn = std::function<bool(Scope<Message>&)>;

#define THREAD_MSG_FN(fn) \
  std::bind_front(&fn , this)

  class Thread {
    public:
      Thread(const ThreadRenderContext& context , const std::string& name , const ConfigTable& config) 
        : config(config) , thread_name(name) , id(FNV(name)) , context(context) {}
      virtual ~Thread() {}

      void Run();
      void Join();

      void RegisterEndpoint(UUID id , Scope<ChannelEndpoint<Message>>& endpoint);

      UUID GetID() const;

    protected:
      ConfigTable config;
      
      std::string thread_name = "UNKNOWN Thread";
      UUID id;

      /// 1u reserved for core channel, since other ids are a FNV hash of the name,
      ///   there will be no collisions
      constexpr static UUID kCoreEndpointId{ 1u };

      std::map<UUID , Scope<ChannelEndpoint<Message>>> endpoints;

      Scope<ChannelEndpoint<Message>>& GetCoreChannel();
      
      ThreadRenderContext context;

    private:
      Scope<std::jthread> thread = nullptr;
      std::mutex init_mutex;
      std::condition_variable init_condition;


      ThreadMessageFn process_msg = THREAD_MSG_FN(Thread::ProcessInitMessage);

      bool main_loop = false;

      virtual void Step() = 0;

      std::vector<Scope<Message>> PollChannels();

      bool ProcessInitMessage(Scope<Message>& msg);
      bool ProcessStartMessage(Scope<Message>& msg);

      bool ProcessMessage(Scope<Message>& msg);

      void SendReadyMessage(bool ready);
      
      void Main(std::stop_token st);

      virtual void Init(Scope<Message>& msg) {}
      virtual void Shutdown() {}
  };

} // namespace other

#endif // !OTHER_ENGINE_THREAD_HPP
