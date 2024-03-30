/**
 * \file event\key_events.hpp
*/
#ifndef OTHER_ENGINE_KEY_EVENTS_HPP
#define OTHER_ENGINE_KEY_EVENTS_HPP

#include <sstream>

#include "event/event.hpp"
#include "input/keyboard.hpp"

namespace other {

  class KeyEvent : public Event {
    protected:
      KeyEvent(Keyboard::Key key_code) : key_code(key_code) {}
      Keyboard::Key key_code;

    public:
      inline Keyboard::Key Key() const { return key_code; }

      EVENT_CATEGORY(KEYBOARD_EVENT | INPUT_EVENT)
  }; 

  class KeyPressed : public KeyEvent {
    public:
      KeyPressed(Keyboard::Key key_code) 
        : KeyEvent(key_code) {}

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "Key Pressed : " << static_cast<uint16_t>(key_code);
        return ss.str();
      }

      EVENT_TYPE(KEY_PRESSED);
  };

  class KeyReleased : public KeyEvent {
    public:
      KeyReleased(Keyboard::Key key_code) 
        : KeyEvent(key_code) {}

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "Key Released : " << static_cast<uint16_t>(key_code);
        return ss.str();
      }

      EVENT_TYPE(KEY_RELEASED);
  };

  class KeyHeld : public KeyEvent {
    uint32_t repeat;

    public:
      KeyHeld(Keyboard::Key key_code , uint32_t repeat)
        : KeyEvent(key_code) , repeat(repeat) {}

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "Key Held : " << static_cast<uint16_t>(key_code);
        return ss.str();
      }

      uint32_t FramesHeld() const { return repeat; }

      EVENT_TYPE(KEY_HELD);
  };

} // namespace other

#endif // !OTHER_ENGINE_KEY_EVENTS_HPP
