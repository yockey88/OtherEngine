/**
 * \file event\window_events.hpp
*/
#ifndef OTHER_ENGINE_WINDOW_EVENTS_HPP
#define OTHER_ENGINE_WINDOW_EVENTS_HPP

#include <sstream>

#include <glm/glm.hpp>

#include "event/event.hpp"

namespace other {

  class WindowEvent : public Event {
    public:
      WindowEvent() {}
      ~WindowEvent() {}

      EVENT_CATEGORY(WINDOW_EVENT | APPLICATION_EVENT | CORE_EVENT);
  };

  class WindowResized : public WindowEvent {
    public:
      WindowResized(const glm::ivec2& size , const glm::ivec2& old_size) 
        : size(size) , old_size(old_size) {}
      ~WindowResized() {}

      glm::ivec2 WindowSize() const { return size; }
      glm::ivec2 OldWindowSize() const { return old_size; }

      uint32_t Width() const { return size.x; }
      uint32_t Height() const { return size.y; }

      uint32_t OldWidth() const { return old_size.x; }
      uint32_t OldHeight() const { return old_size.y; }

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "WindowResizeEvent: " << size.x << ", " << size.y;
        return ss.str();
      }

      EVENT_TYPE(WINDOW_RESIZE);
      
    private:
      glm::ivec2 size;
      glm::ivec2 old_size;
  };

  class WindowMinimized : public WindowEvent {
    public:
      WindowMinimized() {}
      ~WindowMinimized() {}

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "WindowMinimizedEvent";
        return ss.str();
      }

      EVENT_TYPE(WINDOW_MINIMIZE);
  };

  class WindowClosed : public WindowEvent {
    public:
      WindowClosed() {}
      ~WindowClosed() {}

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "WindowClosedEvent";
        return ss.str();
      }

      EVENT_TYPE(WINDOW_CLOSE);
  };

} // namespace other

#endif // !OTHER_ENGINE_WINDOW_EVENTS_HPP
