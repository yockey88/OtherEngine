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
      enum Button : uint8_t {
        LEFT = 1 ,
        MIDDLE = 2 ,
        RIGHT = 3 ,
        X1 = 4 ,
        /// not sure what button this is?
        X2 = 5 ,
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
      static void LockCursor();

      static ButtonState GetButtonState(Button button); 

      static uint32_t GetX();
      static uint32_t GetY();
      
      static uint32_t PreviousX(); 
      static uint32_t PreviousY(); 

      static uint32_t GetDX(); 
      static uint32_t GetDY(); 

      static bool InWindow(); 

      static uint32_t FramesHeld(Button button);
      static bool Pressed(Button button);
      static bool Blocked(Button button);
      static bool Held(Button button);
      static bool Released(Button button);
  };

} // namespace other

#endif // !OTHER_ENGINE_MOUSE_HPP
