/**
 * \file scripting/ecs/lua_bindings.cpp
 **/
#include "scripting/lua/lua_bindings.hpp"

#include <entt/entity/fwd.hpp>
#include <sol/raii.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <entt/entt.hpp>

#include "core/logger.hpp"
#include "input/mouse.hpp"
#include "input/keyboard.hpp"
#include "scripting/lua/native_functions/lua_native_logging.hpp"

namespace other {

  void LuaScriptBindings::InitializeBindings(sol::state &lua_state) {
    BindGlmTypes(lua_state); 

    lua_state["Logger"] = lua_state.create_table();
    lua_state["Logger"]["WriteTrace"] = lua_script_bindings::WriteTrace;
    lua_state["Logger"]["WriteDebug"] = lua_script_bindings::WriteDebug;
    lua_state["Logger"]["WriteInfo"] = lua_script_bindings::WriteInfo;
    lua_state["Logger"]["WriteWarning"] = lua_script_bindings::WriteWarning;
    lua_state["Logger"]["WriteError"] = lua_script_bindings::WriteError;
    lua_state["Logger"]["WriteCritical"] = lua_script_bindings::WriteCritical;

    InitializeKeyEnums(lua_state);

    lua_state.new_usertype<Keyboard>(
      "Keyboard" , 
      "FramesHeld" , Keyboard::FramesHeld ,
      "Pressed" , Keyboard::Pressed , 
      "Blocked" , Keyboard::Blocked , 
      "Held" , Keyboard::Held , 
      "Down" , Keyboard::Down ,
      "Released" , Keyboard::Released
    );

    InitializeMouseEnums(lua_state);

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
      
  void LuaScriptBindings::BindGlmTypes(sol::state& lua_state) {
    auto vec2_multiply = sol::overload(
      [](const glm::vec2& v , float s) -> glm::vec2 { return s * v; } ,
      [](float s , const glm::vec2& v) -> glm::vec2 { return s * v; }
    );

    auto vec2_divide = sol::overload(
      [](const glm::vec2& v , float s) -> glm::vec2 { return v / s; } 
    );

    auto vec2_add = sol::overload(
      [](const glm::vec2& v1 , const glm::vec2& v2) -> glm::vec2 { return v1 + v2; } ,
      [](const glm::vec2& v , float s) -> glm::vec2 { return s + v; } ,
      [](float s , const glm::vec2& v) -> glm::vec2 { return s + v; }
    );

    auto vec2_subtract = sol::overload(
      [](const glm::vec2& v1 , const glm::vec2& v2) -> glm::vec2 { return v1 - v2; } ,
      [](const glm::vec2& v , float s) -> glm::vec2 { return s - v; } ,
      [](float s , const glm::vec2& v) -> glm::vec2 { return s - v; }
    );

    auto vec3_multiply = sol::overload(
      [](const glm::vec3& v , float s) -> glm::vec3 { return s * v; } ,
      [](float s , const glm::vec3& v) -> glm::vec3 { return s * v; }
    );

    auto vec3_divide = sol::overload(
      [](const glm::vec3& v , float s) -> glm::vec3 { return v / s; } 
    );

    auto vec3_add = sol::overload(
      [](const glm::vec3& v1 , const glm::vec3& v2) -> glm::vec3 { return v1 + v2; } ,
      [](const glm::vec3& v , float s) -> glm::vec3 { return s + v; } ,
      [](float s , const glm::vec3& v) -> glm::vec3 { return s + v; }
    );

    auto vec3_subtract = sol::overload(
      [](const glm::vec3& v1 , const glm::vec3& v2) -> glm::vec3 { return v1 - v2; } ,
      [](const glm::vec3& v , float s) -> glm::vec3 { return s - v; } ,
      [](float s , const glm::vec3& v) -> glm::vec3 { return s - v; }
    );
    
    auto vec4_multiply = sol::overload(
      [](const glm::vec4& v , float s) -> glm::vec4 { return s * v; } ,
      [](float s , const glm::vec4& v) -> glm::vec4 { return s * v; }
    );

    auto vec4_divide = sol::overload(
      [](const glm::vec4& v , float s) -> glm::vec4 { return v / s; } 
    );

    auto vec4_add = sol::overload(
      [](const glm::vec4& v1 , const glm::vec4& v2) -> glm::vec4 { return v1 + v2; } ,
      [](const glm::vec4& v , float s) -> glm::vec4 { return s + v; } ,
      [](float s , const glm::vec4& v) -> glm::vec4 { return s + v; }
    );

    auto vec4_subtract = sol::overload(
      [](const glm::vec4& v1 , const glm::vec4& v2) -> glm::vec4 { return v1 - v2; } ,
      [](const glm::vec4& v , float s) -> glm::vec4 { return s - v; } ,
      [](float s , const glm::vec4& v) -> glm::vec4 { return s - v; }
    );
    
    lua_state.new_usertype<glm::vec2>(
      "Vec2" ,
      sol::call_constructor ,
      sol::constructors<glm::vec2(float) , glm::vec2(float , float)>() ,
      "x" , &glm::vec2::x ,
      "y" , &glm::vec2::y ,
      sol::meta_function::multiplication , vec2_multiply ,
      sol::meta_function::division , vec2_divide ,
      sol::meta_function::addition , vec2_add ,
      sol::meta_function::subtraction , vec2_subtract ,
      "length" , [](const glm::vec2& v) -> float { return glm::length(v); } ,
      "length2" , [](const glm::vec2& v) -> float { return glm::length(v) * glm::length(v); } ,
      "normalize" , [](const glm::vec2& v) -> glm::vec2 { return glm::normalize(v); } ,
      "direction_to" , [](const glm::vec2& v1 , const glm::vec2& v2) -> glm::vec2 { return glm::normalize(v2 - v1); } ,
      "nearly_zero_x" , [](const glm::vec2& v) -> bool { return glm::epsilonEqual(v.x , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_y" , [](const glm::vec2& v) -> bool { return glm::epsilonEqual(v.y , 0.f , std::numeric_limits<float>::epsilon()); }
    );

    lua_state.new_usertype<glm::vec3>(
      "Vec3" ,
      sol::call_constructor ,
      sol::constructors<glm::vec3(float) , glm::vec3(float , float , float)>() ,
      "x" , &glm::vec3::x ,
      "y" , &glm::vec3::y ,
      "z" , &glm::vec3::z ,
      sol::meta_function::multiplication , vec3_multiply ,
      sol::meta_function::division , vec3_divide ,
      sol::meta_function::addition , vec3_add ,
      sol::meta_function::subtraction , vec3_subtract ,
      "length" , [](const glm::vec3& v) -> float { return glm::length(v); } ,
      "length2" , [](const glm::vec3& v) -> float { return glm::length(v) * glm::length(v); } ,
      "normalize" , [](const glm::vec3& v) -> glm::vec3 { return glm::normalize(v); } ,
      "direction_to" , [](const glm::vec3& v1 , const glm::vec3& v2) -> glm::vec3 { return glm::normalize(v2 - v1); } ,
      "nearly_zero_x" , [](const glm::vec3& v) -> bool { return glm::epsilonEqual(v.x , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_y" , [](const glm::vec3& v) -> bool { return glm::epsilonEqual(v.y , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_z" , [](const glm::vec3& v) -> bool { return glm::epsilonEqual(v.z , 0.f , std::numeric_limits<float>::epsilon()); }
    );
    
    lua_state.new_usertype<glm::vec4>(
      "Vec4" ,
      sol::call_constructor ,
      sol::constructors<glm::vec4(float) , glm::vec4(float , float , float , float)>() ,
      "x" , &glm::vec4::x ,
      "y" , &glm::vec4::y ,
      "z" , &glm::vec4::z ,
      "w" , &glm::vec4::w ,
      sol::meta_function::multiplication , vec4_multiply ,
      sol::meta_function::division , vec4_divide ,
      sol::meta_function::addition , vec4_add ,
      sol::meta_function::subtraction , vec4_subtract ,
      "length" , [](const glm::vec4& v) -> float { return glm::length(v); } ,
      "length2" , [](const glm::vec4& v) -> float { return glm::length(v) * glm::length(v); } ,
      "normalize" , [](const glm::vec4& v) -> glm::vec4 { return glm::normalize(v); } ,
      "direction_to" , [](const glm::vec4& v1 , const glm::vec4& v2) -> glm::vec4 { return glm::normalize(v2 - v1); } ,
      "nearly_zero_x" , [](const glm::vec4& v) -> bool { return glm::epsilonEqual(v.x , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_y" , [](const glm::vec4& v) -> bool { return glm::epsilonEqual(v.y , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_z" , [](const glm::vec4& v) -> bool { return glm::epsilonEqual(v.z , 0.f , std::numeric_limits<float>::epsilon()); } ,
      "nearly_zero_w" , [](const glm::vec4& v) -> bool { return glm::epsilonEqual(v.w , 0.f , std::numeric_limits<float>::epsilon()); }
    );

    lua_state.set_function("dot2" , [](const glm::vec2& v1 , const glm::vec2& v2) -> float { return glm::dot(v1 , v2); });
    lua_state.set_function("dot3" , [](const glm::vec3& v1 , const glm::vec3& v2) -> float { return glm::dot(v1 , v2); });
    lua_state.set_function("dot4" , [](const glm::vec4& v1 , const glm::vec4& v2) -> float { return glm::dot(v1 , v2); });

    lua_state.set_function("cross" , [](const glm::vec3& v1 , const glm::vec3& v2) -> glm::vec3 { return glm::cross(v1 , v2); });
  }

namespace {

  [[nodiscard]] static inline entt::id_type GetTypeId(const sol::table& obj) {
    const auto f = obj["type_id"].get<sol::function>();
    OE_ASSERT(f.valid() , "TypeId not exposed to LUA");
    return f.valid() ? f().get<entt::id_type>() : -1;
  }

  template <typename... Args>
  static inline auto InvokeMetaFunc(entt::meta_type meta_type , entt::id_type function_id , Args&&... args) {
    if (!meta_type) {

    } else {
      if (auto&& meta_function = meta_type.func(function_id); meta_function) {
        return meta_function.invoke({} , std::forward<Args>(args)...);
      }
    }
    return entt::meta_any{};
  }

  template <typename... Args>
  static inline auto InvokeMetaFunc(entt::id_type type_id , entt::id_type function_id , Args&&... args) {
    return InvokeMetaFunc(entt::resolve(type_id) , function_id , std::forward<Args>(args)...);
  }

} // anonyomous namespace 
      
  void LuaScriptBindings::BindEcsTypes(sol::state& lua_state) {
    using namespace entt::literals;

    lua_state.new_usertype<entt::registry>(
      "registry" , 
      sol::meta_function::construct , sol::factories([]() -> entt::registry { return entt::registry{}; }) ,
      "size" , [](const entt::registry& self) -> size_t { return self.storage<entt::entity>()->size(); } ,
      "alive" , [](const entt::registry& self) -> bool { return self.storage<entt::entity>()->free_list(); } ,
      "valid" , &entt::registry::valid , 
      "current" , &entt::registry::current , 
      "create" , [](entt::registry& self) -> entt::entity { return self.create(); } ,
      "destroy" , [](entt::registry& self , entt::entity handle) { return self.destroy(handle); } // ,
      /// TODO: implement entt::meta reflection bindings to complete binding ecs to lua 
      // "emplace" , [](entt::registry& self , entt::entity entity , const sol::table& comp , sol::this_state s) -> sol::object {
      //   if (!comp.valid()) {
      //     return sol::lua_nil_t{};
      //   }

      //   const auto maybe_any = InvokeMetaFunc(GetTypeId(comp) , "emplace"_hs , &self , entity , comp , s);
      //   return maybe_any ? 
      //     maybe_any.cast<sol::reference>() : 
      //     sol::lua_nil_t{};
      // }
    );
  }
  
  void LuaScriptBindings::InitializeMouseEnums(sol::state& lua_state) {
    lua_state["Button"] = lua_state.create_table_with(
      "First" , Mouse::Button::FIRST , 
      "Left" , Mouse::Button::LEFT , 
      "Middle" , Mouse::Button::MIDDLE ,
      "Right" , Mouse::Button::RIGHT ,
      "X1" , Mouse::Button::X1 ,
      "X2" , Mouse::Button::X2 ,
      "Last" , Mouse::Button::LAST
    );
  }

  void LuaScriptBindings::InitializeKeyEnums(sol::state& lua_state) {
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
      "F13" , Keyboard::Key::OE_F13 ,
      "F14" , Keyboard::Key::OE_F14 ,
      "F15" , Keyboard::Key::OE_F15 ,
      "F16" , Keyboard::Key::OE_F16 ,
      "F17" , Keyboard::Key::OE_F17 ,
      "F18" , Keyboard::Key::OE_F18 ,
      "F19" , Keyboard::Key::OE_F19 ,
      "F20" , Keyboard::Key::OE_F20 ,
      "F21" , Keyboard::Key::OE_F21 ,
      "F22" , Keyboard::Key::OE_F22 ,
      "F23" , Keyboard::Key::OE_F23 ,
      "F24" , Keyboard::Key::OE_F24 ,
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


} // namespace other
