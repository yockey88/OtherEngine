/**
 * \file input\mouse.cpp
*/
#include "input\mouse.hpp"

#include <SDL.h>

#include "event\event_queue.hpp"
#include "event\mouse_events.hpp"

namespace other {

  void Mouse::Initialize() {
    SDL_Window* window = SDL_GetMouseFocus();
    if (window == nullptr) {
      return;
    }

    int32_t x , y;
    SDL_GetWindowSize(window , &x , &y);

    SDL_WarpMouseInWindow(window , x / 2 , y / 2);

    for (uint8_t i = 0; i < kButtonCount; ++i) {
        buttons[static_cast<Button>(i)] = ButtonState{};
    }
  }

  void Mouse::Update() {
    state.previous_position = state.position;

    uint32_t curr_state = SDL_GetMouseState(&state.position.x , &state.position.y);

    SDL_Window* window = SDL_GetMouseFocus();
    state.in_window = window != nullptr;

    for (uint32_t b = 0; b < kButtonCount; ++b) {
      Button button = static_cast<Button>(b);
      ButtonState& button_state = buttons[button];
      
      button_state.previous_state = button_state.current_state;

      if (!(curr_state & SDL_BUTTON(b)) && button_state.current_state == State::PRESSED) {
        button_state.current_state = State::RELEASED;
        button_state.frames_held = 0;

        EventQueue::PushEvent<MouseButtonReleased>(button);
        continue;
      }
      
      if (curr_state & SDL_BUTTON(b) && button_state.current_state == State::RELEASED) {
        button_state.current_state = State::PRESSED;
        ++button_state.frames_held;

        EventQueue::PushEvent<MouseButtonPressed>(button , button_state.frames_held);
        continue;
      }
      
      if (curr_state & SDL_BUTTON(b) && button_state.current_state == State::PRESSED &&
          button_state.frames_held <= 22) {
        button_state.current_state = State::BLOCKED;
        ++button_state.frames_held;

        continue;
      }

      if (curr_state & SDL_BUTTON(b) && button_state.current_state == State::BLOCKED &&
          button_state.frames_held <= 22) {
        ++button_state.frames_held;

        continue;
      }

      if (curr_state & SDL_BUTTON(b) && button_state.current_state == State::BLOCKED && 
          button_state.frames_held > 22) {
        button_state.current_state = State::HELD;
        ++button_state.frames_held;

        continue;
      }

      if (curr_state & SDL_BUTTON(b) && button_state.current_state == State::HELD) {
        ++button_state.frames_held;

        EventQueue::PushEvent<MouseButtonHeld>(button , button_state.frames_held);
        continue;
      }
    }
  }
  
  void Mouse::SnapToCenter() {
    SDL_Window* window = SDL_GetMouseFocus();
    if (window == nullptr)
        return;

    int32_t x , y;
    SDL_GetWindowSize(window , &x , &y);

    SDL_WarpMouseInWindow(window , x / 2 , y / 2);
  }

  void Mouse::FreeCursor() {
    SDL_SetRelativeMouseMode(SDL_FALSE);
  }

  void Mouse::HideCursor() {
    SDL_SetRelativeMouseMode(SDL_TRUE);
  }

} // namespace other
