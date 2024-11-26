/**
 * \file event\mouse_events.hpp
 */
#ifndef OTHER_ENGINE_MOUSE_EVENTS_HPP
#define OTHER_ENGINE_MOUSE_EVENTS_HPP

#include <sstream>

#include <glm/glm.hpp>

#include "event/event.hpp"
#include "input/mouse.hpp"

namespace other {

  struct MouseMoved {
    MouseMoved(const glm::vec2& position, const glm::vec2& last_position)
        : position(position), last_position(last_position) {}

    inline float X() const { return position.x; }
    inline float Y() const { return position.y; }

    inline float PreviousX() const { return last_position.x; }
    inline float PreviousY() const { return last_position.y; }

    std::string ToString() const {
      std::stringstream ss;
      ss << "Mouse Moved : [" << position.x << " , " << position.y << "]";
      return ss.str();
    }

    EVENT_TYPE(MOUSE_MOVD);
    EVENT_CATEGORY(MOUSE_EVNT | INPUT_EVENT);

   private:
    glm::vec2 position;
    glm::vec2 last_position;
  };

  struct MouseScrolled {
    MouseScrolled(const glm::vec2& offset)
        : offset(offset) {}

    inline float X() const { return offset.x; }
    inline float Y() const { return offset.y; }

    std::string ToString() const {
      std::stringstream ss;
      ss << "Mouse Scrolled : [" << offset.x << " , " << offset.y << "]";
      return ss.str();
    }

    EVENT_TYPE(MOUSE_SCROLLED);
    EVENT_CATEGORY(MOUSE_EVNT | INPUT_EVENT);

    glm::vec2 offset;
  };

#define MOUSE_BUTTON_EVENT()                                        \
  EVENT_CATEGORY(MOUSE_BUTTON_EVENT | MOUSE_EVNT | INPUT_EVENT);    \
  inline Mouse::Button Button() const { return button; }            \
  inline bool HasPosition() const { return mouse_pos.has_value(); } \
  inline glm::ivec2 Position() const {                              \
    OE_ASSERT(HasPosition(), "Mouse position not set!");            \
    return mouse_pos.value();                                       \
  }                                                                 \
  Mouse::Button button;                                             \
  Opt<glm::ivec2> mouse_pos;

  struct MouseButtonPressed {
    MOUSE_BUTTON_EVENT();
    EVENT_TYPE(MOUSE_BUTTON_PRESSED);

    std::string ToString() const {
      std::stringstream ss;
      ss << "Mouse Button Pressed :: [" << static_cast<uint16_t>(button) << "]";
      return ss.str();
    }

    uint32_t FramesHeld() const { return repeat; }

    uint32_t repeat;
  };

  struct MouseButtonReleased {
    MOUSE_BUTTON_EVENT();
    EVENT_TYPE(MOUSE_BUTTON_RELEASED)
    std::string ToString() const {
      std::stringstream ss;
      ss << "MouseButtonReleased :: [" << static_cast<uint8_t>(button) << "]";
      return ss.str();
    }
  };

  struct MouseButtonHeld {
    MOUSE_BUTTON_EVENT();
    EVENT_TYPE(MOUSE_BUTTON_HELD);

    std::string ToString() const {
      std::stringstream ss;
      ss << "MouseButtonHeld :: [" << static_cast<uint8_t>(button) << "]";
      return ss.str();
    }

    uint32_t FramesHeld() const { return repeat; }

    uint32_t repeat;
  };

  static_assert(Event<MouseMoved>, "MouseMoved does not meet the Event concept");
  static_assert(Event<MouseScrolled>, "MouseScrolled does not meet the Event concept");
  static_assert(Event<MouseButtonPressed>, "MouseButtonPressed does not meet the Event concept");
  static_assert(Event<MouseButtonReleased>, "MouseButtonReleased does not meet the Event concept");
  static_assert(Event<MouseButtonHeld>, "MouseButtonHeld does not meet the Event concept");

}  // namespace other

#endif  // !OTHER_ENGINE_MOUSE_EVENTS_HPP
