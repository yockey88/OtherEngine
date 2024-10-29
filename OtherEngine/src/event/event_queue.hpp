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
      /// write the event
      size_t idx = event_buffer.BufferData(arg);

      /// write the event handle
      EventHandle handle{
        .ptr = event_buffer.PointerAt<T>(idx),
        .type = T::GetStaticType(),
      };
      scratch_buffer.BufferData(handle);
      ++num_events;
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

      auto itr = event_handlers.find(h);
      if (itr == event_handlers.end()) {
        auto& handler = event_handlers[h] = EventDispatcher{};
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
    static constexpr size_t kBufferSize = 1024 * 1024;
    static inline uint64_t event_flags = 0;
    static inline bool process_ui_events = true;
    static inline size_t num_events = 0;

    static Buffer event_buffer;
    static Buffer scratch_buffer;
    static std::map<uint64_t, EventDispatcher> event_handlers;

    static void SetEventFlag(EventType type);
    static void Dispatch();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EVENT_QUEUE_HPP
