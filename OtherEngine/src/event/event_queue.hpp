/**
 * \file event/event_queue.hpp
*/
#ifndef OTHER_ENGINE_EVENT_QUEUE_HPP
#define OTHER_ENGINE_EVENT_QUEUE_HPP

#include "core/logger.hpp"
#include "event/event.hpp"

namespace other {

  class Engine;
  class App;

  class EventQueue { 
    public:
      static void Initialize();

      static void Poll(Engine* engine , App* app);

      template<event_t T , typename... Args>
      static void PushEvent(Args&&... args) {
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

      static void Dispatch(Engine* engine , App* app_data);
      
      static void Clear();
  };

} // namespace other

#endif // !OTHER_ENGINE_EVENT_QUEUE_HPP
