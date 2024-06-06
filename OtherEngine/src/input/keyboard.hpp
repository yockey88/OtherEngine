/**
 * \file input\keyboard.hpp
*/
#ifndef OTHER_ENGINE_KEYBOARD_HPP
#define OTHER_ENGINE_KEYBOARD_HPP

#include <cstdint>
#include <map>

namespace other {

class Keyboard {
  public:
    enum class Key {
      OE_UNKNOWN = 0 ,
      OE_UNKNOWN1 = 1 ,
      OE_UNKNOWN2 = 2 ,
      OE_UNKNOWN3 = 3 ,

      OE_A = 4,
      OE_B = 5,
      OE_C = 6,
      OE_D = 7,
      OE_E = 8,
      OE_F = 9,
      OE_G = 10,
      OE_H = 11,
      OE_I = 12,
      OE_J = 13,
      OE_K = 14,
      OE_L = 15,
      OE_M = 16,
      OE_N = 17,
      OE_O = 18,
      OE_P = 19,
      OE_Q = 20,
      OE_R = 21,
      OE_S = 22,
      OE_T = 23,
      OE_U = 24,
      OE_V = 25,
      OE_W = 26,
      OE_X = 27,
      OE_Y = 28,
      OE_Z = 29,

      OE_1 = 30,
      OE_2 = 31,
      OE_3 = 32,
      OE_4 = 33,
      OE_5 = 34,
      OE_6 = 35,
      OE_7 = 36,
      OE_8 = 37,
      OE_9 = 38,
      OE_0 = 39,

      OE_RETURN = 40,
      OE_ESCAPE = 41,
      OE_BACKSPACE = 42,
      OE_TAB = 43,
      OE_SPACE = 44,

      OE_MINUS = 45,
      OE_EQUALS = 46,
      OE_LEFTBRACKET = 47,
      OE_RIGHTBRACKET = 48,
      OE_BACKSLASH = 49, 
      OE_NONUSHASH = 50, 
      OE_SEMICOLON = 51,
      OE_APOSTROPHE = 52,
      OE_GRAVE = 53, 
      OE_COMMA = 54,
      OE_PERIOD = 55,
      OE_SLASH = 56,

      OE_CAPSLOCK = 57,

      OE_F1 = 58,
      OE_F2 = 59,
      OE_F3 = 60,
      OE_F4 = 61,
      OE_F5 = 62,
      OE_F6 = 63,
      OE_F7 = 64,
      OE_F8 = 65,
      OE_F9 = 66,
      OE_F10 = 67,
      OE_F11 = 68,
      OE_F12 = 69,

      OE_PRINTSCREEN = 70,
      OE_SCROLLLOCK = 71,
      OE_PAUSE = 72,
      OE_INSERT = 73, 
      OE_HOME = 74,
      OE_PAGEUP = 75,
      OE_DELETE = 76,
      OE_END = 77,
      OE_PAGEDOWN = 78,
      OE_RIGHT = 79,
      OE_LEFT = 80,
      OE_DOWN = 81,
      OE_UP = 82,

      OE_NUMLOCKCLEAR = 83, 
      OE_KP_DIVIDE = 84,
      OE_KP_MULTIPLY = 85,
      OE_KP_MINUS = 86,
      OE_KP_PLUS = 87,
      OE_KP_ENTER = 88,
      OE_KP_1 = 89,
      OE_KP_2 = 90,
      OE_KP_3 = 91,
      OE_KP_4 = 92,
      OE_KP_5 = 93,
      OE_KP_6 = 94,
      OE_KP_7 = 95,
      OE_KP_8 = 96,
      OE_KP_9 = 97,
      OE_KP_0 = 98,
      OE_KP_PERIOD = 99,

      OE_NONUSBACKSLASH = 100, 
      OE_APPLICATION = 101, 
      OE_POWER = 102, 
      OE_KP_EQUALS = 103,
      OE_F13 = 104,
      OE_F14 = 105,
      OE_F15 = 106,
      OE_F16 = 107,
      OE_F17 = 108,
      OE_F18 = 109,
      OE_F19 = 110,
      OE_F20 = 111,
      OE_F21 = 112,
      OE_F22 = 113,
      OE_F23 = 114,
      OE_F24 = 115,
      OE_EXEC = 116,
      OE_HELP = 117,
      OE_MENU = 118,
      OE_SELECT = 119,
      OE_STOP = 120,
      OE_AGAIN = 121,   /**< redo */
      OE_UNDO = 122,
      OE_CUT = 123,
      OE_COPY = 124,
      OE_PASTE = 125,
      OE_FIND = 126,
      OE_MUTE = 127,
      OE_VOLUMEUP = 128,
      OE_VOLUMEDOWN = 129,
    /*     OE_LOCKINGCAPSLOCK = 130,  */
    /*     OE_LOCKINGNUMLOCK = 131, */
    /*     OE_LOCKINGSCROLLLOCK = 132, */
      OE_KP_COMMA = 133,
      OE_KP_EQUALSAS400 = 134,

      OE_INTERNATIONAL1 = 135,
      OE_INTERNATIONAL2 = 136,
      OE_INTERNATIONAL3 = 137, 
      OE_INTERNATIONAL4 = 138,
      OE_INTERNATIONAL5 = 139,
      OE_INTERNATIONAL6 = 140,
      OE_INTERNATIONAL7 = 141,
      OE_INTERNATIONAL8 = 142,
      OE_INTERNATIONAL9 = 143,
      OE_LANG1 = 144,  
      OE_LANG2 = 145,  
      OE_LANG3 = 146, 
      OE_LANG4 = 147, 
      OE_LANG5 = 148,  
      OE_LANG6 = 149, 
      OE_LANG7 = 150, 
      OE_LANG8 = 151, 
      OE_LANG9 = 152, 

      OE_ALTERASE = 153,
      OE_SYSREQ = 154,
      OE_CANCEL = 155,
      OE_CLEAR = 156,
      OE_PRIOR = 157,
      OE_RETURN2 = 158,
      OE_SEPARATOR = 159,
      OE_OUT = 160,
      OE_OPER = 161,
      OE_CLEARAGAIN = 162,
      OE_CRSEL = 163,
      OE_EXSEL = 164,

      OE_KP_00 = 176,
      OE_KP_000 = 177,
      OE_THOUSANDSSEPARATOR = 178,
      OE_DECIMALSEPARATOR = 179,
      OE_CURRENCYUNIT = 180,
      OE_CURRENCYSUBUNIT = 181,
      OE_KP_LEFTPAREN = 182,
      OE_KP_RIGHTPAREN = 183,
      OE_KP_LEFTBRACE = 184,
      OE_KP_RIGHTBRACE = 185,
      OE_KP_TAB = 186,
      OE_KP_BACKSPACE = 187,
      OE_KP_A = 188,
      OE_KP_B = 189,
      OE_KP_C = 190,
      OE_KP_D = 191,
      OE_KP_E = 192,
      OE_KP_F = 193,
      OE_KP_XOR = 194,
      OE_KP_POWER = 195,
      OE_KP_PERCENT = 196,
      OE_KP_LESS = 197,
      OE_KP_GREATER = 198,
      OE_KP_AMPERSAND = 199,
      OE_KP_DBLAMPERSAND = 200,
      OE_KP_VERTICALBAR = 201,
      OE_KP_DBLVERTICALBAR = 202,
      OE_KP_COLON = 203,
      OE_KP_HASH = 204,
      OE_KP_SPACE = 205,
      OE_KP_AT = 206,
      OE_KP_EXCLAM = 207,
      OE_KP_MEMSTORE = 208,
      OE_KP_MEMRECALL = 209,
      OE_KP_MEMCLEAR = 210,
      OE_KP_MEMADD = 211,
      OE_KP_MEMSUBTRACT = 212,
      OE_KP_MEMMULTIPLY = 213,
      OE_KP_MEMDIVIDE = 214,
      OE_KP_PLUSMINUS = 215,
      OE_KP_CLEAR = 216,
      OE_KP_CLEARENTRY = 217,
      OE_KP_BINARY = 218,
      OE_KP_OCTAL = 219,
      OE_KP_DECIMAL = 220,
      OE_KP_HEXADECIMAL = 221,

      OE_LCTRL = 224,
      OE_LSHIFT = 225,
      OE_LALT = 226, 
      OE_LGUI = 227, 
      OE_RCTRL = 228,
      OE_RSHIFT = 229,
      OE_RALT = 230,
      OE_RGUI = 231,

      OE_MODE = 257, 

      OE_AUDIONEXT = 258,
      OE_AUDIOPREV = 259,
      OE_AUDIOSTOP = 260,
      OE_AUDIOPLAY = 261,
      OE_AUDIOMUTE = 262,
      OE_MEDIASELECT = 263,
      OE_WWW = 264,
      OE_MAIL = 265,
      OE_CALCULATOR = 266,
      OE_COMPUTER = 267,
      OE_AC_SEARCH = 268,
      OE_AC_HOME = 269,
      OE_AC_BACK = 270,
      OE_AC_FORWARD = 271,
      OE_AC_STOP = 272,
      OE_AC_REFRESH = 273,
      OE_AC_BOOKMARKS = 274,

      OE_BRIGHTNESSDOWN = 275,
      OE_BRIGHTNESSUP = 276,
      OE_DISPLAYSWITCH = 277, 
      OE_KBDILLUMTOGGLE = 278,
      OE_KBDILLUMDOWN = 279,
      OE_KBDILLUMUP = 280,
      OE_EJECT = 281,
      OE_SLEEP = 282,

      OE_APP1 = 283,
      OE_APP2 = 284,

      OE_AUDIOREWIND = 285,
      OE_AUDIOFASTFORWARD = 286,

      OE_SOFTLEFT = 287, 
      OE_SOFTRIGHT = 288, 
      OE_CALL = 289, 
      OE_ENDCALL = 290, 

      OE_NUM_YES = 512 
    };

    enum class State : uint8_t {
      PRESSED = 0 ,
      BLOCKED ,
      HELD ,
      RELEASED
    };

    struct KeyState {
      State current_state = State::RELEASED;
      State previous_state = State::RELEASED;
      uint32_t frames_held = 0;
      uint32_t frames_blocked = 0;
    };

    static void Initialize();
    static void Update();

    static KeyState GetKeyState(Key key);

    static bool Pressed(Key key);
    static bool Blocked(Key key);
    static bool Held(Key key);
    static bool KeyDown(Key key);
    static bool Released(Key key);

    static inline bool LCtrlLayer() { 
      return Blocked(Key::OE_LCTRL) || Held(Key::OE_LCTRL);
    }
    static inline bool RCtrlLayer() { 
      return Blocked(Key::OE_RCTRL) || Held(Key::OE_RCTRL);
    }

    static inline bool LAltLayer() { 
      return Blocked(Key::OE_LALT) || Held(Key::OE_LALT);
    }
    static inline bool RAltLayer() { 
      return Blocked(Key::OE_RALT) || Held(Key::OE_RALT);
    }

    static inline bool LCtrlShiftLayer() { 
      return LCtrlLayer() && (Blocked(Key::OE_LSHIFT) || Held(Key::OE_LSHIFT));
    }
    static inline bool RCtrlShiftLayer() { 
      return RCtrlLayer() && (Blocked(Key::OE_RSHIFT) || Held(Key::OE_RSHIFT));
    }

    static inline bool LCtrlAltLayer() { 
      return LCtrlLayer() && (Blocked(Key::OE_LALT) || Held(Key::OE_LALT));
    }
    static inline bool RCtrlAltLayer() { 
      return RCtrlLayer() && (Blocked(Key::OE_RALT) || Held(Key::OE_RALT));
    }

    static inline bool LCtrlAltShiftLayer() { 
      return LCtrlLayer() && LAltLayer() && (Blocked(Key::OE_LSHIFT) || Held(Key::OE_LSHIFT));
    }
    static inline bool RCtrlAltShiftLayer() { 
      return RCtrlLayer() && RAltLayer() && (Blocked(Key::OE_RSHIFT) || Held(Key::OE_RSHIFT));
    }

    static inline bool LCtrlLayerKey(Key key) { 
      return LCtrlLayer() && Pressed(key);
    }
    static inline bool RCtrlLayerKey(Key key)  { 
      return RCtrlLayer() && Pressed(key); 
    }

    static inline bool LAltLayerKey(Key key) { 
      return LAltLayer() && Pressed(key);
    }   
    static inline bool RAltLayerKey(Key key) { 
      return RAltLayer() && Pressed(key);
    }

    static inline bool LCtrlShiftLayerKey(Key key) { 
      return LCtrlShiftLayer() && Pressed(key); 
    }
    static inline bool RCtrlShiftLayerKey(Key key) { 
      return RCtrlShiftLayer() && Pressed(key);
    }

    static inline bool LCtrlAltLayerKey(Key key) { 
      return LCtrlAltLayer() && Pressed(key);
    }
    static inline bool RCtrlAltLayerKey(Key key) { 
      return RCtrlAltLayer() && Pressed(key);
    }
    
    static inline bool LCtrlAltShiftLayerKey(Key key) { 
      return LCtrlAltShiftLayer() && Pressed(key);
    }

    static inline bool RCtrlAltShiftLayerKey(Key key) { 
      return RCtrlAltShiftLayer() && Pressed(key);
    }

  private:
    constexpr static uint16_t kKeyCount = 287;
    /// TODO: find some way to make this configurable
    constexpr static uint32_t key_block_length = 5000;
    constexpr static uint32_t key_repeat_length = 1000;
    
    static uint8_t* state;
    static std::map<Key , KeyState> keys;
  };

} // namespace other

namespace std {

  template <>
  struct hash<other::Keyboard::Key> {
    size_t operator()(const other::Keyboard::Key& key) const {
      return static_cast<size_t>(key);
    }
  };

} // namespace std

#endif // !OTHER_ENGINE_KEYBOARD_HPP
