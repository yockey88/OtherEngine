/**
 * \file event/ui_events.hpp
 **/
#ifndef OTHER_ENGINE_UI_EVENTS_HPP
#define OTHER_ENGINE_UI_EVENTS_HPP

#include "core/uuid.hpp"

#include "event/event.hpp"

namespace other {

  class UIEvent : public Event {
    public:
      virtual ~UIEvent() override {}

      EVENT_CATEGORY(UI_EVENT | APPLICATION_EVENT);
  };

  class UIWindowClosed : public UIEvent {
    public:
      UIWindowClosed(UUID window_id) 
        : id(window_id) {}
      virtual ~UIWindowClosed() override {}

      UUID GetWindowId() const { return id; }

      EVENT_TYPE(UI_WINDOW_CLOSE);

    private:
      UUID id;
  };

} // namespace other 

#endif // !OTHER_ENGINE_UI_EVENTS_HPP
