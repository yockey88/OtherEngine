/**
 * \file core/channel.hpp
 **/
#ifndef OTHER_ENGINE_CHANNEL_HPP
#define OTHER_ENGINE_CHANNEL_HPP

#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "core/defines.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "thread/message.hpp"

namespace other {

  template <MsgType M>
  struct ChannelQueue : public RefCounted {
    std::mutex mutex;
    std::condition_variable condition;
    std::queue<Scope<M>> queue;
  };

  template <MsgType M>
  class Sender {
    public:
      Sender(Ref<ChannelQueue<M>> queue)
        : queue(Ref<ChannelQueue<M>>::Clone(queue)) {}

      void Send(Scope<M>& msg) {
        std::lock_guard<std::mutex> lock(queue->mutex);
        queue->queue.push(std::move(msg));
        queue->condition.notify_one();
      }

    private:
      Ref<ChannelQueue<M>> queue;
  };

  template <MsgType M>
  class Receiver {
    public: 
      Receiver(Ref<ChannelQueue<M>> queue)
        : queue(Ref<ChannelQueue<M>>::Clone(queue)) {}

      Scope<M> Receive(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(queue->mutex);
        if (timeout.count() == 0) {
          queue->condition.wait(lock , [this]() { return !queue->queue.empty(); });
        } else {
          queue->condition.wait_for(lock , timeout , [this]() { return !queue->queue.empty(); });
        }

        if (queue->queue.empty()) {
          return nullptr;
        }

        Scope<M> msg = std::move(queue->queue.front());
        queue->queue.pop();

        return msg;
      }

    private:
      Ref<ChannelQueue<M>> queue;
  };

  template <MsgType M>
  class ChannelEndpoint : public RefCounted {
    public:
      ChannelEndpoint(Scope<Sender<M>> sender , Scope<Receiver<M>> receiver) 
        : sender(std::move(sender)) , receiver(std::move(receiver)) {}

      void Send(Scope<M>& msg) { sender->Send(msg); }
      Scope<M> Receive(std::chrono::milliseconds timeout) { return receiver->Receive(timeout); }

    private:
      Scope<Sender<M>> sender;
      Scope<Receiver<M>> receiver;
  };

  template <MsgType M>
  using Channel = std::pair<Scope<ChannelEndpoint<M>> , Scope<ChannelEndpoint<M>>>;

  template <MsgType M>
  Channel<M> GetChannel() {
    Ref<ChannelQueue<M>> queue1 = NewRef<ChannelQueue<M>>();
    Ref<ChannelQueue<M>> queue2 = NewRef<ChannelQueue<M>>();

    Scope<Sender<M>> sender1 = NewScope<Sender<M>>(queue1);
    Scope<Sender<M>> sender2 = NewScope<Sender<M>>(queue2);

    Scope<Receiver<M>> receiver1 = NewScope<Receiver<M>>(queue1);
    Scope<Receiver<M>> receiver2 = NewScope<Receiver<M>>(queue2);

    Scope<ChannelEndpoint<M>> ep1 = NewScope<ChannelEndpoint<M>>(std::move(sender1) , std::move(receiver2));
    Scope<ChannelEndpoint<M>> ep2 = NewScope<ChannelEndpoint<M>>(std::move(sender2) , std::move(receiver1));

    return Channel<M>{ std::move(ep1) , std::move(ep2) };
  }

} // namespace other

#endif // !OTHER_ENGINE_CHANNEL_HPP
