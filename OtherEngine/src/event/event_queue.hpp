/**
 * \file event/event_queue.hpp
*/
#ifndef OTHER_ENGINE_EVENT_QUEUE_HPP
#define OTHER_ENGINE_EVENT_QUEUE_HPP

#include "core/config.hpp"
#include "core/logger.hpp"
#include "event/event.hpp"

namespace other {

  class Engine;
  class App;

  class EventQueue { 
    public:
      static void Initialize(const ConfigTable& config);

      static void Poll(App* app);
      static void Clear();

      template<event_t T , typename... Args>
      static void PushEvent(Args&&... args) {
        // OE_ASSERT(std::is_trivially_copyable_v<T> , "Event type [{}] is not trivially copyable" , typeid(T).name());
        OE_ASSERT(sizeof(T) + buffer_offset < buffer_size , "Event buffer overflow");

        T event(std::forward<Args>(args)...);
        memcpy(cursor , &event , sizeof(T));
        cursor += sizeof(T);
        buffer_offset += sizeof(T);
      }

      static void EnableUIEvents();
      static void DisableUIEvents();

      static void Shutdown();

    private:
      constexpr static uint32_t buffer_size = 4096;
      static uint32_t buffer_offset;
      static uint8_t* event_buffer;
      static uint8_t* cursor;

      static bool process_ui_events;

      static void Dispatch(App* app_data);
      
  };

} // namespace other

#endif // !OTHER_ENGINE_EVENT_QUEUE_HPP
