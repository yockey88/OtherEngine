/**
 * \file event\key_events.hpp
 */
#ifndef OTHER_ENGINE_KEY_EVENTS_HPP
#define OTHER_ENGINE_KEY_EVENTS_HPP

#include <sstream>

#include "event/event.hpp"
#include "input/keyboard.hpp"

namespace other {

#define KEY_EVENT()                                     \
  EVENT_CATEGORY(KEYBOARD_EVENT | INPUT_EVENT);         \
  inline Keyboard::Key Key() const { return key_code; } \
  Keyboard::Key key_code;

  struct KeyPressed {
    KEY_EVENT();
    EVENT_TYPE(KEY_PRESSED);

    std::string ToString() const {
      std::stringstream ss;
      ss << "Key Pressed : " << static_cast<uint16_t>(key_code);
      return ss.str();
    }
  };

  struct KeyReleased {
    KEY_EVENT();
    EVENT_TYPE(KEY_RELEASED);

    std::string ToString() const {
      std::stringstream ss;
      ss << "Key Released : " << static_cast<uint16_t>(key_code);
      return ss.str();
    }
  };

  struct KeyHeld {
    KEY_EVENT();
    EVENT_TYPE(KEY_HELD);

    std::string ToString() const {
      std::stringstream ss;
      ss << "Key Held : " << static_cast<uint16_t>(key_code);
      return ss.str();
    }

    uint32_t FramesHeld() const { return repeat; }

    uint32_t repeat;
  };

  static_assert(Event<KeyPressed>, "KeyPressed does not meet the Event concept");
  static_assert(Event<KeyReleased>, "KeyReleased does not meet the Event concept");
  static_assert(Event<KeyHeld>, "KeyHeld does not meet the Event concept");

  template <typename KE, typename Fn>
    requires(std::same_as<KE, KeyPressed> ||
             std::same_as<KE, KeyReleased> ||
             std::same_as<KE, KeyHeld>) &&
    requires(KE& e, Fn f) {
      f();
      { f() } -> std::same_as<void>;
    }
  static void HandleKeyEvent(KE& event, Keyboard::Key key, Fn fn) {
    if (event.key_code == key) {
      fn();
    }
  }

}  // namespace other

#endif  // !OTHER_ENGINE_KEY_EVENTS_HPP
