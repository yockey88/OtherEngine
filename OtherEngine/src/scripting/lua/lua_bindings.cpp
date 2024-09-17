/**
 * \file scripting/ecs/lua_bindings.cpp
 **/
#include "scripting/lua/lua_bindings.hpp"

#include <sol/raii.hpp>
#include <entt/entt.hpp>

#include "core/logger.hpp"
#include "input/mouse.hpp"
#include "input/keyboard.hpp"

#include "scripting/lua/native_functions/lua_native_logging.hpp"
#include "scripting/lua/lua_math_bindings.hpp"
#include "scripting/lua/lua_component_bindings.hpp"
#include "scripting/lua/lua_scene_bindings.hpp"
#include "scripting/lua/lua_ui_bindings.hpp"

namespace other {
namespace lua_script_bindings {

  void BindKeyEnums(sol::state& lua_state);

  void BindCoreTypes(sol::state& lua_state) {
    lua_state["Logger"] = lua_state.create_table();
    lua_state["Logger"]["WriteTrace"] = lua_script_bindings::WriteTrace;
    lua_state["Logger"]["WriteDebug"] = lua_script_bindings::WriteDebug;
    lua_state["Logger"]["WriteInfo"] = lua_script_bindings::WriteInfo;
    lua_state["Logger"]["WriteWarning"] = lua_script_bindings::WriteWarning;
    lua_state["Logger"]["WriteError"] = lua_script_bindings::WriteError;
    lua_state["Logger"]["WriteCritical"] = lua_script_bindings::WriteCritical;

    lua_state["Button"] = lua_state.create_table_with(
      "Left" , Mouse::Button::LEFT , 
      "Middle" , Mouse::Button::MIDDLE ,
      "Right" , Mouse::Button::RIGHT ,
      "X1" , Mouse::Button::X1 ,
      "X2" , Mouse::Button::X2
    );

    lua_state.new_usertype<Keyboard>(
      "Keyboard" , 
      "FramesHeld" , Keyboard::FramesHeld ,
      "Pressed" , Keyboard::Pressed , 
      "Blocked" , Keyboard::Blocked , 
      "Held" , Keyboard::Held , 
      "Down" , Keyboard::Down ,
      "Released" , Keyboard::Released
    );

    BindKeyEnums(lua_state);

    lua_state.new_usertype<Mouse>(
      "Mouse" ,
      "SnapToCenter" , Mouse::SnapToCenter ,
      "FreeCursor" , Mouse::FreeCursor , 
      "LockCursor" , Mouse::LockCursor ,
      "GetX" , Mouse::GetX ,
      "GetY" , Mouse::GetY ,
      "PreviousX" , Mouse::PreviousX ,
      "PreviousY" , Mouse::PreviousY , 
      "GetDX" , Mouse::GetDX , 
      "GetDY" , Mouse::GetDY ,
      "InWindow" , Mouse::InWindow ,
      "FramesHeld" , Mouse::FramesHeld ,
      "Pressed" , Mouse::Blocked ,
      "Held" , Mouse::Held ,
      "Released" , Mouse::Released ,
      "MousePos" , []() -> glm::vec2 { return { Mouse::GetX() , Mouse::GetY() }; } , 
      "MousePreviousPos" , []() -> glm::vec2 { return { Mouse::PreviousX() , Mouse::PreviousY() }; } , 
      "MouseDeltaPos" , []() -> glm::vec2 { return { Mouse::GetDX() , Mouse::GetDY() }; }
    );
  }
  
  void BindAll(sol::state& lua_state) {
    BindGlmTypes(lua_state);
    BindCoreTypes(lua_state);
    BindEcsTypes(lua_state);
    BindScene(lua_state);
    BindUiTypes(lua_state);
  }

namespace {

  // [[nodiscard]] static inline entt::id_type GetTypeId(const sol::table& obj) {
  //   const auto f = obj["type_id"].get<sol::function>();
  //   OE_ASSERT(f.valid() , "TypeId not exposed to LUA");
  //   return f.valid() ? f().get<entt::id_type>() : -1;
  // }

  // template <typename... Args>
  // static inline auto InvokeMetaFunc(entt::meta_type meta_type , entt::id_type function_id , Args&&... args) {
  //   if (!meta_type) {

  //   } else {
  //     if (auto&& meta_function = meta_type.func(function_id); meta_function) {
  //       return meta_function.invoke({} , std::forward<Args>(args)...);
  //     }
  //   }
  //   return entt::meta_any{};
  // }

  // template <typename... Args>
  // static inline auto InvokeMetaFunc(entt::id_type type_id , entt::id_type function_id , Args&&... args) {
  //   return InvokeMetaFunc(entt::resolve(type_id) , function_id , std::forward<Args>(args)...);
  // }

} // anonyomous namespace 
      
  void BindKeyEnums(sol::state& lua_state) {
    lua_state["Key"] = lua_state.create_table_with(
      "A" , Keyboard::Key::OE_A ,
      "B" , Keyboard::Key::OE_B ,
      "C" , Keyboard::Key::OE_C ,
      "D" , Keyboard::Key::OE_D ,
      "E" , Keyboard::Key::OE_E ,
      "F" , Keyboard::Key::OE_F ,
      "G" , Keyboard::Key::OE_G ,
      "H" , Keyboard::Key::OE_H ,
      "I" , Keyboard::Key::OE_I ,
      "J" , Keyboard::Key::OE_J ,
      "K" , Keyboard::Key::OE_K ,
      "L" , Keyboard::Key::OE_L ,
      "M" , Keyboard::Key::OE_M ,
      "N" , Keyboard::Key::OE_N ,
      "O" , Keyboard::Key::OE_O ,
      "P" , Keyboard::Key::OE_P ,
      "Q" , Keyboard::Key::OE_Q ,
      "R" , Keyboard::Key::OE_R ,
      "S" , Keyboard::Key::OE_S ,
      "T" , Keyboard::Key::OE_T ,
      "U" , Keyboard::Key::OE_U ,
      "V" , Keyboard::Key::OE_V ,
      "W" , Keyboard::Key::OE_W ,
      "X" , Keyboard::Key::OE_X ,
      "Y" , Keyboard::Key::OE_Y ,
      "Z" , Keyboard::Key::OE_Z ,
      "1" , Keyboard::Key::OE_1 ,
      "2" , Keyboard::Key::OE_2 ,
      "3" , Keyboard::Key::OE_3 ,
      "4" , Keyboard::Key::OE_4 ,
      "5" , Keyboard::Key::OE_5 ,
      "6" , Keyboard::Key::OE_6 ,
      "7" , Keyboard::Key::OE_7 ,
      "8" , Keyboard::Key::OE_8 ,
      "9" , Keyboard::Key::OE_9 ,
      "0" , Keyboard::Key::OE_0 ,
      "Return" , Keyboard::Key::OE_RETURN ,
      "Escape" , Keyboard::Key::OE_ESCAPE ,
      "Backspace" , Keyboard::Key::OE_BACKSPACE ,
      "Tab" , Keyboard::Key::OE_TAB ,
      "Space" , Keyboard::Key::OE_SPACE ,
      "Minus" , Keyboard::Key::OE_MINUS ,
      "Equals" , Keyboard::Key::OE_EQUALS ,
      "LeftBracket" , Keyboard::Key::OE_LEFTBRACKET ,
      "RightBracket" , Keyboard::Key::OE_RIGHTBRACKET ,
      "BackSlash" , Keyboard::Key::OE_BACKSLASH  ,
      "NoNusHash" , Keyboard::Key::OE_NONUSHASH  ,
      "SemiColon" , Keyboard::Key::OE_SEMICOLON ,
      "Apostrophe" , Keyboard::Key::OE_APOSTROPHE ,
      "Grave" , Keyboard::Key::OE_GRAVE  ,
      "Comma" , Keyboard::Key::OE_COMMA ,
      "Period" , Keyboard::Key::OE_PERIOD ,
      "Slash" , Keyboard::Key::OE_SLASH ,
      "Capslock" , Keyboard::Key::OE_CAPSLOCK ,
      "F1" , Keyboard::Key::OE_F1 ,
      "F2" , Keyboard::Key::OE_F2 ,
      "F3" , Keyboard::Key::OE_F3 ,
      "F4" , Keyboard::Key::OE_F4 ,
      "F5" , Keyboard::Key::OE_F5 ,
      "F6" , Keyboard::Key::OE_F6 ,
      "F7" , Keyboard::Key::OE_F7 ,
      "F8" , Keyboard::Key::OE_F8 ,
      "F9" , Keyboard::Key::OE_F9 ,
      "F10" , Keyboard::Key::OE_F10 ,
      "F11" , Keyboard::Key::OE_F11 ,
      "F12" , Keyboard::Key::OE_F12 ,
      // "PrintScreen" , Keyboard::Key::OE_PRINTSCREEN ,
      // "ScrollLock" , Keyboard::Key::OE_SCROLLLOCK ,
      // "Pause" , Keyboard::Key::OE_PAUSE ,
      // "Insert" , Keyboard::Key::OE_INSERT  ,
      // "Home" , Keyboard::Key::OE_HOME ,
      // "PageUp" , Keyboard::Key::OE_PAGEUP ,
      // "Delete" , Keyboard::Key::OE_DELETE ,
      // "End" , Keyboard::Key::OE_END ,
      // "PageDown" , Keyboard::Key::OE_PAGEDOWN ,
      "Right" , Keyboard::Key::OE_RIGHT ,
      "Left" , Keyboard::Key::OE_LEFT ,
      "Down" , Keyboard::Key::OE_DOWN ,
      "Up" , Keyboard::Key::OE_UP ,
      // "NumLockClear" , Keyboard::Key::OE_NUMLOCKCLEAR  ,
      // "KpDivide" , Keyboard::Key::OE_KP_DIVIDE ,
      // "KpMultiply" , Keyboard::Key::OE_KP_MULTIPLY ,
      // "KpMinus" , Keyboard::Key::OE_KP_MINUS ,
      // "KpPlus" , Keyboard::Key::OE_KP_PLUS ,
      // "KpEnter" , Keyboard::Key::OE_KP_ENTER ,
      // "Kp1" , Keyboard::Key::OE_KP_1 ,
      // "Kp2" , Keyboard::Key::OE_KP_2 ,
      // "Kp3" , Keyboard::Key::OE_KP_3 ,
      // "Kp4" , Keyboard::Key::OE_KP_4 ,
      // "Kp5" , Keyboard::Key::OE_KP_5 ,
      // "Kp6" , Keyboard::Key::OE_KP_6 ,
      // "Kp7" , Keyboard::Key::OE_KP_7 ,
      // "Kp8" , Keyboard::Key::OE_KP_8 ,
      // "Kp9" , Keyboard::Key::OE_KP_9 ,
      // "Kp0" , Keyboard::Key::OE_KP_0 ,
      // "KpPeriod" , Keyboard::Key::OE_KP_PERIOD ,
      // "NoNusBackslash" , Keyboard::Key::OE_NONUSBACKSLASH  ,
      // "Application" , Keyboard::Key::OE_APPLICATION  ,
      // "Power" , Keyboard::Key::OE_POWER  ,
      // "KpEquals" , Keyboard::Key::OE_KP_EQUALS ,
      // "F13" , Keyboard::Key::OE_F13 ,
      // "F14" , Keyboard::Key::OE_F14 ,
      // "F15" , Keyboard::Key::OE_F15 ,
      // "F16" , Keyboard::Key::OE_F16 ,
      // "F17" , Keyboard::Key::OE_F17 ,
      // "F18" , Keyboard::Key::OE_F18 ,
      // "F19" , Keyboard::Key::OE_F19 ,
      // "F20" , Keyboard::Key::OE_F20 ,
      // "F21" , Keyboard::Key::OE_F21 ,
      // "F22" , Keyboard::Key::OE_F22 ,
      // "F23" , Keyboard::Key::OE_F23 ,
      // "F24" , Keyboard::Key::OE_F24 ,
    //   "Exec" , Keyboard::Key::OE_EXEC ,
    //   "Help" , Keyboard::Key::OE_HELP ,
    //   "Menu" , Keyboard::Key::OE_MENU ,
    //   "Select" , Keyboard::Key::OE_SELECT ,
    //   "Stop" , Keyboard::Key::OE_STOP ,
    //   "Again" , Keyboard::Key::OE_AGAIN ,
    //   "Undo" , Keyboard::Key::OE_UNDO ,
    //   "Cut" , Keyboard::Key::OE_CUT ,
    //   "Copy" , Keyboard::Key::OE_COPY ,
    //   "Paste" , Keyboard::Key::OE_PASTE ,
    //   "Find" , Keyboard::Key::OE_FIND ,
    //   "Mute" , Keyboard::Key::OE_MUTE ,
    //   "VolumeUp" , Keyboard::Key::OE_VOLUMEUP ,
    //   "VolumeDown" , Keyboard::Key::OE_VOLUMEDOWN ,
    //   "KpComma" , Keyboard::Key::OE_KP_COMMA ,
    //   "KpEqualsAs400" , Keyboard::Key::OE_KP_EQUALSAS400 ,
    //   "International1" , Keyboard::Key::OE_INTERNATIONAL1 ,
    //   "International2" , Keyboard::Key::OE_INTERNATIONAL2 ,
    //   "International3" , Keyboard::Key::OE_INTERNATIONAL3  ,
    //   "International4" , Keyboard::Key::OE_INTERNATIONAL4 ,
    //   "International5" , Keyboard::Key::OE_INTERNATIONAL5 ,
    //   "International6" , Keyboard::Key::OE_INTERNATIONAL6 ,
    //   "International7" , Keyboard::Key::OE_INTERNATIONAL7 ,
    //   "International8" , Keyboard::Key::OE_INTERNATIONAL8 ,
    //   "International9" , Keyboard::Key::OE_INTERNATIONAL9 ,
    //   "Lang1" , Keyboard::Key::OE_LANG1  ,
    //   "Lang2" , Keyboard::Key::OE_LANG2  ,
    //   "Lang3" , Keyboard::Key::OE_LANG3  ,
    //   "Lang4" , Keyboard::Key::OE_LANG4  ,
    //   "Lang5" , Keyboard::Key::OE_LANG5  ,
    //   "Lang6" , Keyboard::Key::OE_LANG6  ,
    //   "Lang7" , Keyboard::Key::OE_LANG7  ,
    //   "Lang8" , Keyboard::Key::OE_LANG8  ,
    //   "Lang9" , Keyboard::Key::OE_LANG9  ,
    //   "AltErase" , Keyboard::Key::OE_ALTERASE ,
    //   "SysReq" , Keyboard::Key::OE_SYSREQ ,
    //   "Cancel" , Keyboard::Key::OE_CANCEL ,
    //   "Clear" , Keyboard::Key::OE_CLEAR ,
    //   "Prior" , Keyboard::Key::OE_PRIOR ,
    //   "Return2" , Keyboard::Key::OE_RETURN2 ,
    //   "Seperator" , Keyboard::Key::OE_SEPARATOR ,
    //   "Out" , Keyboard::Key::OE_OUT ,
    //   "Oper" , Keyboard::Key::OE_OPER ,
    //   "ClearAgain" , Keyboard::Key::OE_CLEARAGAIN ,
    //   "Crsel" , Keyboard::Key::OE_CRSEL ,
    //   "Exsel" , Keyboard::Key::OE_EXSEL ,
    //   "Kp00" , Keyboard::Key::OE_KP_00 ,
    //   "Kp000" , Keyboard::Key::OE_KP_000 ,
    //   "ThousandSeparator" , Keyboard::Key::OE_THOUSANDSSEPARATOR ,
    //   "DecimalSeparator" , Keyboard::Key::OE_DECIMALSEPARATOR ,
    //   "CurrencyUnit" , Keyboard::Key::OE_CURRENCYUNIT ,
    //   "CurrencyUbUnit" , Keyboard::Key::OE_CURRENCYSUBUNIT ,
    //   "KpLeftParen" , Keyboard::Key::OE_KP_LEFTPAREN ,
    //   "KpRightParen" , Keyboard::Key::OE_KP_RIGHTPAREN ,
    //   "KpLeftBrace" , Keyboard::Key::OE_KP_LEFTBRACE ,
    //   "KpRightBrace" , Keyboard::Key::OE_KP_RIGHTBRACE ,
    //   "KpTab" , Keyboard::Key::OE_KP_TAB ,
    //   "KpBackspace" , Keyboard::Key::OE_KP_BACKSPACE ,
    //   "KpA" , Keyboard::Key::OE_KP_A ,
    //   "KpB" , Keyboard::Key::OE_KP_B ,
    //   "KpC" , Keyboard::Key::OE_KP_C ,
    //   "KpD" , Keyboard::Key::OE_KP_D ,
    //   "KpE" , Keyboard::Key::OE_KP_E ,
    //   "KpF" , Keyboard::Key::OE_KP_F ,
    //   "KpXor" , Keyboard::Key::OE_KP_XOR ,
    //   "KpPower" , Keyboard::Key::OE_KP_POWER ,
    //   "KpPercent" , Keyboard::Key::OE_KP_PERCENT ,
    //   "KpLess" , Keyboard::Key::OE_KP_LESS ,
    //   "KpGreater" , Keyboard::Key::OE_KP_GREATER ,
    //   "KpAmpersand" , Keyboard::Key::OE_KP_AMPERSAND ,
    //   "KpDblAmpersand" , Keyboard::Key::OE_KP_DBLAMPERSAND ,
    //   "KpVerticalBar" , Keyboard::Key::OE_KP_VERTICALBAR ,
    //   "KpDblVerticalBar" , Keyboard::Key::OE_KP_DBLVERTICALBAR ,
    //   "KpColon" , Keyboard::Key::OE_KP_COLON ,
    //   "KpHash" , Keyboard::Key::OE_KP_HASH ,
    //   "KpSpace" , Keyboard::Key::OE_KP_SPACE ,
    //   "KpAt" , Keyboard::Key::OE_KP_AT ,
    //   "KpExclam" , Keyboard::Key::OE_KP_EXCLAM ,
    //   "KpMemStore" , Keyboard::Key::OE_KP_MEMSTORE ,
    //   "KpMemRecall" , Keyboard::Key::OE_KP_MEMRECALL ,
    //   "KpMemClear" , Keyboard::Key::OE_KP_MEMCLEAR ,
    //   "KpMemAdd" , Keyboard::Key::OE_KP_MEMADD ,
    //   "KpMemSubtract" , Keyboard::Key::OE_KP_MEMSUBTRACT ,
    //   "KpMemMultiply" , Keyboard::Key::OE_KP_MEMMULTIPLY ,
    //   "KpMemDivide" , Keyboard::Key::OE_KP_MEMDIVIDE ,
    //   "KpPlusMinus" , Keyboard::Key::OE_KP_PLUSMINUS ,
    //   "KpClear" , Keyboard::Key::OE_KP_CLEAR ,
    //   "KpClearEntry" , Keyboard::Key::OE_KP_CLEARENTRY ,
    //   "KpBinary" , Keyboard::Key::OE_KP_BINARY ,
    //   "KpOctal" , Keyboard::Key::OE_KP_OCTAL ,
    //   "KpDecimal" , Keyboard::Key::OE_KP_DECIMAL ,
    //   "KpHexaDecimal" , Keyboard::Key::OE_KP_HEXADECIMAL ,
      "LCtrl" , Keyboard::Key::OE_LCTRL ,
      "LShift" , Keyboard::Key::OE_LSHIFT ,
      "LAlt" , Keyboard::Key::OE_LALT  ,
      "LGui" , Keyboard::Key::OE_LGUI  ,
      "RCtrl" , Keyboard::Key::OE_RCTRL ,
      "RShift" , Keyboard::Key::OE_RSHIFT ,
      "RAlt" , Keyboard::Key::OE_RALT // ,
    //   "RGui" , Keyboard::Key::OE_RGUI ,
    //   "Mode" , Keyboard::Key::OE_MODE  ,
    //   "AudioNext" , Keyboard::Key::OE_AUDIONEXT ,
    //   "AudioPrev" , Keyboard::Key::OE_AUDIOPREV ,
    //   "AudioStop" , Keyboard::Key::OE_AUDIOSTOP ,
    //   "AudioPlay" , Keyboard::Key::OE_AUDIOPLAY ,
    //   "AudioMute" , Keyboard::Key::OE_AUDIOMUTE ,
    //   "MediaSelect" , Keyboard::Key::OE_MEDIASELECT ,
    //   "WWW" , Keyboard::Key::OE_WWW ,
    //   "Mail" , Keyboard::Key::OE_MAIL ,
    //   "Calculator" , Keyboard::Key::OE_CALCULATOR ,
    //   "Computer" , Keyboard::Key::OE_COMPUTER ,
    //   "AcSearch" , Keyboard::Key::OE_AC_SEARCH ,
    //   "AcHome" , Keyboard::Key::OE_AC_HOME ,
    //   "AcBack" , Keyboard::Key::OE_AC_BACK ,
    //   "AcForward" , Keyboard::Key::OE_AC_FORWARD ,
    //   "AcStop" , Keyboard::Key::OE_AC_STOP ,
    //   "AcRefresh" , Keyboard::Key::OE_AC_REFRESH ,
    //   "AcBookmarks" , Keyboard::Key::OE_AC_BOOKMARKS ,
    //   "BrightnessDown" , Keyboard::Key::OE_BRIGHTNESSDOWN ,
    //   "BrightnessUp" , Keyboard::Key::OE_BRIGHTNESSUP ,
    //   "DisplaySwitch" , Keyboard::Key::OE_DISPLAYSWITCH  ,
    //   "KbDillumToggle" , Keyboard::Key::OE_KBDILLUMTOGGLE ,
    //   "KbDillumDown" , Keyboard::Key::OE_KBDILLUMDOWN ,
    //   "KbDillumUp" , Keyboard::Key::OE_KBDILLUMUP ,
    //   "Eject" , Keyboard::Key::OE_EJECT ,
    //   "Sleep" , Keyboard::Key::OE_SLEEP ,
    //   "App1" , Keyboard::Key::OE_APP1 ,
    //   "App2" , Keyboard::Key::OE_APP2 ,
    //   "AudioRewind" , Keyboard::Key::OE_AUDIOREWIND ,
    //   "AudioFastForward" , Keyboard::Key::OE_AUDIOFASTFORWARD ,
    //   "SoftLeft" , Keyboard::Key::OE_SOFTLEFT  ,
    //   "SoftRight" , Keyboard::Key::OE_SOFTRIGHT  ,
    //   "Call" , Keyboard::Key::OE_CALL  ,
    //   "EndCall" , Keyboard::Key::OE_ENDCALL ,
    //   "NumYes" , Keyboard::Key::OE_NUM_YES 
    );
  }

} // namespace lua_script_bindings
} // namespace othe
