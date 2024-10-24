/**
 * \file event/event_queue.hpp
 */
#ifndef OTHER_ENGINE_EVENT_QUEUE_HPP
#define OTHER_ENGINE_EVENT_QUEUE_HPP

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

    static void Poll(App* app);
    static void Clear();

    template <event_t T, typename... Args>
    static void PushEvent(Args&&... args) {
      OE_ASSERT(curr_idx + 1 < kBufferSize, "Event buffer overflow");

      delete event_buffer[curr_idx];
      event_buffer[curr_idx] = new T(std::forward<Args>(args)...);
      ++curr_idx;
    }

    template <event_t E>
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

    static void EnableUIEvents();
    static void DisableUIEvents();

    static void Shutdown();

   private:
    static constexpr size_t kBufferSize = 1024 * 1024;
    static std::array<Event*, kBufferSize> event_buffer;
    static std::map<uint64_t, EventDispatcher> event_handlers;

    static inline uint32_t curr_idx = 0;
    static inline uint64_t event_flags = 0;

    static bool process_ui_events;

    static void SetEventFlag(EventType type);

    static void Dispatch(App* app_data);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EVENT_QUEUE_HPP
