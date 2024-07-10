/**
 * \file input\mouse.cpp
*/
#include "input\mouse.hpp"

#include <SDL.h>

#include "event\event_queue.hpp"
#include "event\mouse_events.hpp"

namespace other {

  Mouse::MouseState Mouse::state = {};
  std::map<Mouse::Button , Mouse::ButtonState> Mouse::buttons = {};
  Opt<Mouse::MouseState> Mouse::pre_free_state = std::nullopt; 

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
      
      if (curr_state & SDL_BUTTON(b) && button_state.current_state == State::RELEASED) {
        button_state.current_state = State::PRESSED;
        button_state.frames_held = 1;

        EventQueue::PushEvent<MouseButtonPressed>(button , button_state.frames_held);
        continue;
      }
      
      if (curr_state & SDL_BUTTON(b) && (button_state.current_state == State::PRESSED ||
          button_state.current_state == State::BLOCKED) && button_state.frames_held <= 22) {
        button_state.current_state = State::BLOCKED;
        ++button_state.frames_held;

        EventQueue::PushEvent<MouseButtonPressed>(button , button_state.frames_held);
        continue;
      } 
      
      if ((curr_state & SDL_BUTTON(b)) && button_state.current_state == State::BLOCKED && 
          button_state.frames_held > 22) {
        button_state.current_state = State::HELD;
        ++button_state.frames_held;

        EventQueue::PushEvent<MouseButtonHeld>(button , button_state.frames_held);
        continue;
      }
      
      if (curr_state & SDL_BUTTON(b) && button_state.current_state == State::HELD) {
        ++button_state.frames_held;

        EventQueue::PushEvent<MouseButtonHeld>(button , button_state.frames_held);
        continue;
      }
      
      if (!(curr_state & SDL_BUTTON(b)) && (button_state.current_state == State::BLOCKED ||
          button_state.current_state == State::PRESSED || button_state.current_state == State::HELD)) {
        button_state.current_state = State::RELEASED;
        button_state.frames_held = 0;

        EventQueue::PushEvent<MouseButtonReleased>(button);
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
    pre_free_state = state;
    SDL_SetRelativeMouseMode(SDL_FALSE);
  }

  void Mouse::LockCursor() {
    if (pre_free_state.has_value()) {
      SDL_Window* window = SDL_GetMouseFocus();
      SDL_WarpMouseInWindow(window , pre_free_state.value().position.x , pre_free_state.value().position.y);
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);
  }

  Mouse::ButtonState Mouse::GetButtonState(Button button) { 
    return buttons[button]; 
  }
      
  glm::ivec2 Mouse::GetPos() {
    int x, y;
    SDL_GetGlobalMouseState(&x, &y);

    state.position = { 
      x , y 
    };

    return state.position;
  }

  glm::ivec2 Mouse::GetRelPos() {
    int rel_x, rel_y;
    SDL_GetRelativeMouseState(&rel_x, &rel_y);

    state.rel_position = {
      rel_x , rel_y
    };

    return state.rel_position;
  }

  uint32_t Mouse::GetX() { 
    return state.position.x; 
  }

  uint32_t Mouse::GetY() { 
    return state.position.y; 
  }
  
  uint32_t Mouse::PreviousX() { 
    return state.previous_position.x; 
  }

  uint32_t Mouse::PreviousY() { 
    return state.previous_position.y; 
  }

  uint32_t Mouse::GetDX() { 
    return state.position.x - state.previous_position.x; 
  }

  uint32_t Mouse::GetDY() { 
    return state.position.y - state.previous_position.y; 
  }

  bool Mouse::InWindow() { 
    return state.in_window; 
  }

  uint32_t Mouse::FramesHeld(Button button) { 
    return buttons[button].frames_held; 
  }

  bool Mouse::Pressed(Button button) { 
    return buttons[button].current_state == State::PRESSED; 
  }

  bool Mouse::Blocked(Button button) { 
    return buttons[button].current_state == State::BLOCKED; 
  }

  bool Mouse::Held(Button button) { 
    return buttons[button].current_state == State::HELD; 
  }

  bool Mouse::Released(Button button) { 
    return buttons[button].current_state == State::RELEASED; 
  }

} // namespace other
