/**
 * \file input/keyboard.cpp
*/
#include "input/keyboard.hpp"

#include <SDL.h>

#include "event/event_queue.hpp"
#include "event/key_events.hpp"

namespace other {

  uint8_t* Keyboard::state = nullptr;
  std::map<Keyboard::Key , Keyboard::KeyState> Keyboard::keys = {};

  void Keyboard::Initialize() {
    for (uint16_t i = 0 ; i < kKeyCount ; ++i)
      keys[static_cast<Key>(i)] = KeyState{};
  }

  void Keyboard::Update() {
    const uint8_t* state = SDL_GetKeyboardState(nullptr);

    for (uint16_t k = 0; k < kKeyCount; ++k) {
      Key key = static_cast<Key>(k);
      KeyState& key_state = keys[key];
      key_state.previous_state = key_state.current_state;

      if (!state[k] && 
          (key_state.current_state == State::PRESSED || 
           key_state.current_state == State::BLOCKED || 
           key_state.current_state == State::HELD)) {
        key_state.current_state = State::RELEASED;
        key_state.frames_held = 0;

        EventQueue::PushEvent<KeyReleased>(key);

        continue;
      }

      if (state[k] && key_state.current_state == State::RELEASED) {
        key_state.current_state = State::PRESSED;
        EventQueue::PushEvent<KeyPressed>(key);

        continue;
      }

      if (state[k] && key_state.current_state == State::PRESSED && key_state.frames_held <= key_block_length) {
        key_state.current_state = State::BLOCKED;
        ++key_state.frames_held;

        continue;
      }

      if (state[k] && key_state.current_state == State::BLOCKED) {
        if (key_state.frames_held > key_block_length) {
          key_state.current_state = State::HELD;
          key_state.frames_held = 0;
        } else {
          ++key_state.frames_held;
        }

        continue;
      }

      if (key_state.current_state == State::HELD) {
        ++key_state.frames_held;
        EventQueue::PushEvent<KeyHeld>(key , key_state.frames_held);

        continue;
      }
    }
  }

  Keyboard::KeyState Keyboard::GetKeyState(Key key) { 
    return keys[key]; 
  }
    
  int32_t Keyboard::FramesHeld(Key key) {
    return keys[key].frames_held;
  }

  bool Keyboard::Pressed(Key key) { 
    return keys[key].current_state == State::PRESSED; 
  }

  bool Keyboard::Blocked(Key key) { 
    return keys[key].current_state == State::BLOCKED; 
  }

  bool Keyboard::Held(Key key) { 
    return keys[key].current_state == State::HELD; 
  }

  bool Keyboard::Down(Key key) { 
    return keys[key].current_state == State::PRESSED || 
      keys[key].current_state == State::BLOCKED ||
      keys[key].current_state == State::HELD; 
  }

  bool Keyboard::Released(Key key) {
    return keys[key].current_state == State::RELEASED; 
  }

} // namespace other
