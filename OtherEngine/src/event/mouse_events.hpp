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

  class MouseMoved : public Event {
    public:
      MouseMoved(const glm::vec2& position , const glm::vec2& last_position)
        : position(position) , last_position(last_position) {}

      inline float X() const { return position.x; }
      inline float Y() const { return position.y; }

      inline float PreviousX() const { return last_position.x; }
      inline float PreviousY() const { return last_position.y; }

      virtual std::string ToString() const override {
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

  class MouseScrolled : public Event {
    public:
      MouseScrolled(const glm::vec2& offset) 
        : offset(offset) {}

      inline float X() const { return offset.x; }
      inline float Y() const { return offset.y; }

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "Mouse Scrolled : [" << offset.x << " , " << offset.y << "]";
        return ss.str();
      }

      EVENT_TYPE(MOUSE_SCROLLED);
      EVENT_CATEGORY(MOUSE_EVNT | INPUT_EVENT);

    public:
      glm::vec2 offset;
  };

  class MouseButton : public Event {
    protected:
      MouseButton(Mouse::Button button) 
        : button(button) {}

      Mouse::Button button;

    public:
      inline Mouse::Button Button() const { return button; }

      EVENT_CATEGORY(MOUSE_BUTTON_EVENT | MOUSE_EVNT | INPUT_EVENT);
  };

  class MouseButtonPressed : public MouseButton {
    public:
      MouseButtonPressed(Mouse::Button button , uint32_t repeat) 
        : MouseButton(button) , repeat(repeat) {}

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "Mouse Button Pressed :: [" << static_cast<uint16_t>(button) << "]";
        return ss.str();
      }

      uint32_t FramesHeld() const { return repeat; }

      EVENT_TYPE(MOUSE_BUTTON_PRESSED);

    private:
      uint32_t repeat;
  };

  class MouseButtonReleased : public MouseButton {
    public:
      MouseButtonReleased(Mouse::Button button)
        : MouseButton(button) {}
  
      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseButtonReleased :: [" << static_cast<uint8_t>(button) << "]";
        return ss.str();
      }
  
      EVENT_TYPE(MOUSE_BUTTON_RELEASED)
  };
  
  class MouseButtonHeld : public MouseButton {
    public:
      MouseButtonHeld(Mouse::Button button , uint32_t repeat)
        : MouseButton(button) , repeat(repeat) {}
  
      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseButtonHeld :: [" << static_cast<uint8_t>(button) << "]";
        return ss.str();
      }

      uint32_t FramesHeld() const { return repeat; }
  
      EVENT_TYPE(MOUSE_BUTTON_HELD);
    
    private:
        uint32_t repeat;
  };

} // namespace other

#endif // !OTHER_ENGINE_MOUSE_EVENTS_HPP
