/**
 * \file thread/channel.hpp
 */
#ifndef OTHER_ENGINE_CHANNEL_HPP
#define OTHER_ENGINE_CHANNEL_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

#include "core/defines.hpp"
#include "core/ref.hpp"

namespace other {

  template <typename T>
  struct Queue {
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable condition;
  };

  template <typename T>
  class Sender {
    public:
      Sender(Ref<Queue<T>>& queue) 
        : queue(queue) {}
      ~Sender() {}

      void Send(const T& msg) {
        std::lock_guard<std::mutex> lock(queue->mutex);
        queue->queue.push(msg);
        queue->condition.notify_one();
      }

      void Close() {
        closed = true;
        queue->condition.notify_one();
      }

    private:
      Ref<Queue<T>> queue;

      bool closed = false;
  };

  template <typename T>
  class Receiver {
    public:
      Receiver(Ref<Queue<T>>& queue) 
        : queue(queue) {}
      ~Receiver() {}

      T Recv() {
        std::unique_lock<std::mutex> lock(queue->mutex);
        queue->condition.wait(lock, [&]{ return !queue->queue.empty(); });
        T item = queue->queue.front();
        queue->queue.pop();
        return item;
      }

      void Close() {
        closed = true;
        queue->condition.notify_one();
      }

      bool Empty() {
        std::lock_guard<std::mutex> lock(queue->mutex);
        return queue->queue.empty();
      }

    private:
      Ref<Queue<T>> queue;

      bool closed = false;
  };

  template <typename T>
  class ChannelEndpoint {
    public:
      ChannelEndpoint(Scope<Sender<T>>& tx , Scope<Receiver<T>>& rx) 
        : tx(std::move(tx)) , rx(std::move(rx)) {}
      ~ChannelEndpoint() {}

      ChannelEndpoint(ChannelEndpoint&&) = delete;
      ChannelEndpoint(const ChannelEndpoint&) = delete;
      ChannelEndpoint& operator=(ChannelEndpoint&&) = delete;
      ChannelEndpoint& operator=(const ChannelEndpoint&) = delete;

      void Send(const T& msg) {
        tx->Send(msg); 
      }

      T Recv() {
        return rx->Recv();
      }
      
      bool Empty() {
        return rx->Empty();
      }

    private:
      Scope<Sender<T>> tx;
      Scope<Receiver<T>> rx;
  };

  template <typename T>
  using Channel = std::pair<Scope<ChannelEndpoint<T>> , Scope<ChannelEndpoint<T>>>;

  template <typename T>
  Channel<T> CreateChannel() {
    Ref<Queue<T>> queue1 = NewRef<Queue<T>>();
    Ref<Queue<T>> queue2 = NewRef<Queue<T>>();

    Scope<Sender<T>> sender1 = NewScope<Sender<T>>(queue1);
    Scope<Sender<T>> sender2 = NewScope<Sender<T>>(queue2);

    Scope<Receiver<T>> receiver1 = NewScope<Receiver<T>>(queue1);
    Scope<Receiver<T>> receiver2 = NewScope<Receiver<T>>(queue2);

    Scope<ChannelEndpoint<T>> channel1 = NewScope<ChannelEndpoint<T>>(sender1 , receiver2);
    Scope<ChannelEndpoint<T>> channel2 = NewScope<ChannelEndpoint<T>>(sender2 , receiver1);

    return std::make_pair(std::move(channel1) , std::move(channel2));
  }

} // namespace other

#endif // !OTHER_ENGINE_CHANNEL_HPP
