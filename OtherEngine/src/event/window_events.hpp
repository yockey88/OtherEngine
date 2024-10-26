/**
 * \file event\window_events.hpp
 */
#ifndef OTHER_ENGINE_WINDOW_EVENTS_HPP
#define OTHER_ENGINE_WINDOW_EVENTS_HPP

#include <sstream>

#include <glm/glm.hpp>

#include "event/event.hpp"

namespace other {

  class WindowEvent {
   public:
    WindowEvent() {}
    ~WindowEvent() {}

    EVENT_CATEGORY(WINDOW_EVENT | APPLICATION_EVENT | CORE_EVENT);
  };

#define WINDOW_EVENT() EVENT_CATEGORY(WINDOW_EVENT | APPLICATION_EVENT | CORE_EVENT);

  struct WindowResized {
    WINDOW_EVENT();
    EVENT_TYPE(WINDOW_RESIZE);

    glm::ivec2 WindowSize() const { return size; }
    glm::ivec2 OldWindowSize() const { return old_size; }

    uint32_t Width() const { return size.x; }
    uint32_t Height() const { return size.y; }

    uint32_t OldWidth() const { return old_size.x; }
    uint32_t OldHeight() const { return old_size.y; }

    std::string ToString() const {
      std::stringstream ss;
      ss << "WindowResizeEvent: " << size.x << ", " << size.y;
      return ss.str();
    }

    glm::ivec2 size;
    glm::ivec2 old_size;
  };

  struct WindowMinimized {
    WINDOW_EVENT();
    EVENT_TYPE(WINDOW_MINIMIZE);

    std::string ToString() const {
      std::stringstream ss;
      ss << "WindowMinimizedEvent";
      return ss.str();
    }
  };

  struct WindowClosed {
    WINDOW_EVENT();
    EVENT_TYPE(WINDOW_CLOSE);

    std::string ToString() const {
      std::stringstream ss;
      ss << "WindowClosedEvent";
      return ss.str();
    }
  };

  static_assert(Event<WindowResized>, "WindowResized does not meet the Event concept");
  static_assert(Event<WindowMinimized>, "WindowMinimized does not meet the Event concept");
  static_assert(Event<WindowClosed>, "WindowClosed does not meet the Event concept");

}  // namespace other

#endif  // !OTHER_ENGINE_WINDOW_EVENTS_HPP
