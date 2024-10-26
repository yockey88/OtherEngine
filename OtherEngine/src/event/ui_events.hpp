/**
 * \file event/ui_events.hpp
 **/
#ifndef OTHER_ENGINE_UI_EVENTS_HPP
#define OTHER_ENGINE_UI_EVENTS_HPP

#include "core/uuid.hpp"

#include "event/event.hpp"

namespace other {

  struct UIWindowClosed {
    UUID GetWindowId() const { return id; }

    EVENT_CATEGORY(UI_EVENT | WINDOW_EVENT | APPLICATION_EVENT | CORE_EVENT);
    EVENT_TYPE(UI_WINDOW_CLOSE);

    uint64_t id;
  };

  static_assert(Event<UIWindowClosed>, "UIWindowClosed does not meet the Event concept");

}  // namespace other

#endif  // !OTHER_ENGINE_UI_EVENTS_HPP
