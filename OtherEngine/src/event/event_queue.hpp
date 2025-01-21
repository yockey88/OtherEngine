/**
 * \file event/event_queue.hpp
 */
#ifndef OTHER_ENGINE_EVENT_QUEUE_HPP
#define OTHER_ENGINE_EVENT_QUEUE_HPP

#include "core/buffer.hpp"
#include "core/config.hpp"
#include "core/logger.hpp"
#include "core/ref.hpp"

#include "event/event.hpp"
#include "event/event_handler.hpp"

namespace other {

  class Engine;
  class App;

  struct EventDispatcher {
    uint64_t hash = 0;
    std::string name = "<invalid>";
    Ref<Dispatcher> dispatcher = nullptr;
  };

  class EventQueue {
   public:
    static void Initialize(const ConfigTable& config);

    static void Poll();
    static void Clear();

    template <typename T>
      requires Event<T>
    static void PushEvent(const T& arg) {
      OE_ASSERT(instance != nullptr, "EventQueue not initialized");
      PROFILE_SECTION("EventQueue--PushEvent");
      size_t idx = instance->event_buffer.BufferData(arg);
      EventHandle handle{
        .ptr = instance->event_buffer.PointerAt<T>(idx),
        .type = T::GetStaticType(),
      };
      instance->scratch_buffer.BufferData(handle);

      ++instance->num_events;
    }

    template <typename T>
      requires Event<T>
    static void PushEvent() {
      PushEvent<T>(T{});
    }

    template <typename E>
      requires Event<E>
    static void RegisterEventDispatcher(const std::string_view name, const std::vector<Handler<E>>& fns) {
      uint64_t h = FNV(name);

      auto itr = instance->event_handlers.find(h);
      if (itr == instance->event_handlers.end()) {
        auto& handler = instance->event_handlers[h] = EventDispatcher{};
        handler.hash = h;
        handler.name = name;
        handler.dispatcher = Ref<DispatchInvoker<E>>::Create(fns);
      } else {
        OE_WARN("Event dispatcher [{}] already registered", name);
      }
    }

    static void UnregisterEventDispatcher(const std::string_view name);

    static void EnableUIEvents();
    static void DisableUIEvents();

    static void Shutdown();

   private:
    friend class ArenaAllocator<EventQueue>;
    static ArenaAllocator<EventQueue> allocator;
    static EventQueue* instance;

    EventQueue() {}
    ~EventQueue() {}

    EventQueue(EventQueue&&) = delete;
    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(EventQueue&&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;

    constexpr static size_t kBufferSize = 1024 * 1024;
    uint64_t event_flags = 0;
    bool process_ui_events = true;

    size_t num_events = 0;
    Buffer event_buffer;
    Buffer scratch_buffer;
    std::map<uint64_t, EventDispatcher> event_handlers;

    static void SetEventFlag(EventType type);
    static void Dispatch();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EVENT_QUEUE_HPP
