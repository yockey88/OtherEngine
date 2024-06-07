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
      constexpr static uint32_t kButtonCount = 5;

      static MouseState state;
      static std::map<Button , ButtonState> buttons;

    public:
      static void Initialize();
      static void Update();
      static void SnapToCenter();
      static void FreeCursor();
      static void HideCursor();

      static inline ButtonState GetButtonState(Button button) { return buttons[button]; }

      static inline uint32_t X() { return state.position.x; }
      static inline uint32_t Y() { return state.position.y; }
      
      static inline uint32_t PreviousX() { return state.previous_position.x; }
      static inline uint32_t PreviousY() { return state.previous_position.y; }

      static inline uint32_t DX() { return state.position.x - state.previous_position.x; }
      static inline uint32_t DY() { return state.position.y - state.previous_position.y; }

      static inline bool InWindow() { return state.in_window; }

      static inline uint32_t FramesHeld(Button button) { return buttons[button].frames_held; }
      static inline bool Pressed(Button button) { return buttons[button].current_state == State::PRESSED; }
      static inline bool Blocked(Button button) { return buttons[button].current_state == State::BLOCKED; }
      static inline bool Held(Button button) { return buttons[button].current_state == State::HELD; }
      static inline bool Released(Button button) { return buttons[button].current_state == State::RELEASED; }
  };

} // namespace other

#endif // !OTHER_ENGINE_MOUSE_HPP
