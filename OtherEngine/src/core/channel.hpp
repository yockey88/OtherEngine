/**
 * \file core/channel.hpp
 **/
#ifndef OTHER_ENGINE_CHANNEL_HPP
#define OTHER_ENGINE_CHANNEL_HPP

#include <vector>
#include <concepts>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "core/defines.hpp"

namespace other {

  /// 4 bytes
  struct MessageMetadata {
    uint32_t size;
  };

  class Message {
    public:
      template <typename T>
      Message(MessageMetadata metadata , T data) 
          : metadata(metadata) {
        uint8_t* data_bytes = reinterpret_cast<uint8_t*>(&data);
        for (size_t i = 0; i < sizeof(T); ++i) {
          bytes.push_back(data_bytes[i]);
        }
      }

      ~Message() {}

      MessageMetadata GetMeta() {
        return metadata;
      }

      template <typename T>
      T& Get() {
        return *reinterpret_cast<T>(bytes.data());
      }

    private:
      MessageMetadata metadata;
      std::vector<uint8_t> bytes;
  };

  template <typename T>
    concept MsgType = std::derived_from<T , Message>;

  template <MsgType M>
  struct ChannelQueue {
    std::mutex mutex;
    std::condition_variable condition;
    std::queue<Scope<M>> queue;
  };

  template <MsgType M>
  class Sender {
    public:
      Sender(Scope<ChannelQueue<M>>& queue)
        : queue(queue) {}

      void Send(Scope<M>& msg) {
        std::unique_lock<std::mutex> lock(queue->mutex);
        queue->queue.push(std::move(msg));
        queue->condition.notify_one();
      }

    private:
      Scope<ChannelQueue<M>>& queue;
  };

  template <MsgType M>
  class Receiver {
    public: 
      Receiver(Scope<ChannelQueue<M>>& queue)
        : queue(queue) {}

      Scope<M> Receive() {
        std::unique_lock<std::mutex> lock(queue->mutex);
        queue->condition.wait(lock , [this]() { return !queue->queue.empty(); });
        Scope<M> msg = std::move(queue->queue.front());
        queue->queue.pop();

        return msg;
      }

    private:
      Scope<ChannelQueue<M>>& queue;
  };

  template <MsgType M>
  class ChannelEndpoint {
    public:
      ChannelEndpoint(Scope<Sender<M>>& sender , Scope<Receiver<M>>& receiver) 
        : sender(sender) , receiver(receiver) {}

      void Send(Scope<M>& msg) { sender->Send(msg); }
      Scope<M> Receive() { return receiver->Receive(); }

    private:
      Scope<Sender<M>>& sender;
      Scope<Receiver<M>>& receiver;
  };

  template <MsgType M>
  class Channel {
    public:
      Channel() {
        queue1 = NewScope<ChannelQueue<M>>();
        queue2 = NewScope<ChannelQueue<M>>();

        sender1 = NewScope<Sender<M>>(queue1);
        sender2 = NewScope<Sender<M>>(queue2);

        receiver1 = NewScope<Receiver<M>>(queue1);
        receiver2 = NewScope<Receiver<M>>(queue2);

        ep1 = NewScope<ChannelEndpoint<M>>(sender1 , receiver2);
        ep2 = NewScope<ChannelEndpoint<M>>(sender2 , receiver1);
      }

      Scope<ChannelEndpoint<M>>& GetFirstEndpoint() {
        return ep1;
      }
      
      Scope<ChannelEndpoint<M>>& GetSecondEndpoint() {
        return ep2;
      }

    private:
      Scope<ChannelQueue<M>> queue1 = nullptr;
      Scope<ChannelQueue<M>> queue2 = nullptr;

      Scope<Sender<M>> sender1 = nullptr;
      Scope<Receiver<M>> receiver1 = nullptr;
      
      Scope<Sender<M>> sender2 = nullptr;
      Scope<Receiver<M>> receiver2 = nullptr;
      
      Scope<ChannelEndpoint<M>> ep1 = nullptr;
      Scope<ChannelEndpoint<M>> ep2 = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_CHANNEL_HPP
