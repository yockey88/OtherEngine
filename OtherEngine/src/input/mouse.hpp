/**
 * \file input\mouse.hpp
*/
#ifndef OTHER_ENGINE_MOUSE_HPP
#define OTHER_ENGINE_MOUSE_HPP

#include <cstdint>
#include <map>

#include <glm/glm.hpp>

namespace other {

  class Mouse {
    public:
      enum class Button : uint8_t {
        FIRST = 0 ,
        LEFT = FIRST ,
        MIDDLE = 1 ,
        RIGHT = 2 ,
        X1 = 3 ,
        X2 = 4 ,
        LAST = X2
      };
      
      enum class State : uint8_t {
        PRESSED = 0 ,
        BLOCKED ,
        HELD ,
        RELEASED
      };

      struct MouseState {
        glm::ivec2 position = glm::ivec2(0 , 0);
        glm::ivec2 previous_position = glm::ivec2(0 , 0);
        bool in_window = false;
      };

      struct ButtonState {
        State current_state = State::RELEASED;
        State previous_state = State::RELEASED;
        uint32_t frames_held = 0;
      };

    private:
      const uint32_t kButtonCount = 5;

      MouseState state;
      std::map<Button , ButtonState> buttons;

    public:
      void Initialize();
      void Update();
      void SnapToCenter();
      void FreeCursor();
      void HideCursor();

      inline ButtonState GetButtonState(Button button) { return buttons[button]; }

      inline uint32_t X() { return state.position.x; }
      inline uint32_t Y() { return state.position.y; }
      
      inline uint32_t PreviousX() { return state.previous_position.x; }
      inline uint32_t PreviousY() { return state.previous_position.y; }

      inline uint32_t DX() { return state.position.x - state.previous_position.x; }
      inline uint32_t DY() { return state.position.y - state.previous_position.y; }

      inline bool InWindow() { return state.in_window; }


      inline uint32_t FramesHeld(Button button) { return buttons[button].frames_held; }
      inline bool Pressed(Button button) { return buttons[button].current_state == State::PRESSED; }
      inline bool Blocked(Button button) { return buttons[button].current_state == State::BLOCKED; }
      inline bool Held(Button button) { return buttons[button].current_state == State::HELD; }
      inline bool Released(Button button) { return buttons[button].current_state == State::RELEASED; }
  };

} // namespace other

#endif // !OTHER_ENGINE_MOUSE_HPP
