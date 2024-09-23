/**
 * \file scripting/lua/lua_ui_bindings.cpp
 **/
#include "scripting/lua/lua_ui_bindings.hpp"

#include <imgui/imgui.h>

#include "core/logger.hpp"

namespace other {
namespace lua_script_bindings {
namespace {

  static bool Begin(const std::string& name) {
    return ImGui::Begin(name.c_str());
  }

  static std::tuple<bool , bool> Begin(const std::string& name , bool is_open) {
    if (!is_open) {
      return std::make_tuple(false, false);
    }
    
    bool can_draw = ImGui::Begin(name.c_str(), &is_open);
    if(!is_open) {
      ImGui::End();
      return std::make_tuple(false, false);
    }
    
    return std::make_tuple(is_open, can_draw);
  }

  static std::tuple<bool , bool> Begin(const std::string& name , bool is_open , int32_t flags) {
    if (!is_open) {
      return std::make_tuple(false , false);
    }

    bool can_draw = ImGui::Begin(name.c_str() , &is_open , static_cast<ImGuiWindowFlags_>(flags));
    if (!is_open) {
      ImGui::End();
      return std::make_tuple(false , false);
    }

    return std::make_tuple(is_open , can_draw);
  }
  
  static void End() {
    ImGui::End(); 
  }
  
  static void SetNextWindowSize(float size_x, float size_y) { 
    ImGui::SetNextWindowSize({ size_x, size_y });
  }
	
  static void SetNextWindowSize(float size_x, float size_y, int cond) { 
    ImGui::SetNextWindowSize({ size_x, size_y }, static_cast<ImGuiCond>(cond)); 
  }
    
  static void SetNextWindowSizeConstraints(float min_x, float min_y, float max_x, float max_y) { 
    ImGui::SetNextWindowSizeConstraints({ min_x, min_y }, { max_x, max_y });
  }
  
  static std::tuple<float, float> GetContentRegionMax() { 
    const auto vec2{ ImGui::GetContentRegionMax() };  
    return std::make_tuple(vec2.x, vec2.y); 
  }
  
  static std::tuple<float, float> GetContentRegionAvail() { 
    const auto vec2{ ImGui::GetContentRegionAvail() };  
    return std::make_tuple(vec2.x, vec2.y); 
  }

  static std::tuple<float, float> GetWindowContentRegionMin() { 
    const auto vec2{ ImGui::GetWindowContentRegionMin() };  
    return std::make_tuple(vec2.x, vec2.y); 
  }
  
  static std::tuple<float, float> GetWindowContentRegionMax() { 
    const auto vec2{ ImGui::GetWindowContentRegionMax() };  
    return std::make_tuple(vec2.x, vec2.y); 
  }

  static void Text(const std::string& text) {
    ImGui::Text("%s" , text.c_str());
  }

  static bool Button(const std::string& label) {
    OE_DEBUG("Button");
    return ImGui::Button(label.c_str());
  }

  // static bool Button(const std::string& label , float size_x , float size_y) {
  //   return ImGui::Button(label.c_str() , { size_x , size_y });
  // }

} // anonymous namespace

  void BindUiTypes(sol::state& lua_state) {
    lua_state.new_enum(
      "ImGuiWindowFlags" , 
      "None"			  , ImGuiWindowFlags_None ,
      "NoTitleBar"		  , ImGuiWindowFlags_NoTitleBar ,
      "NoResize"		  , ImGuiWindowFlags_NoResize ,
      "NoMove"			  , ImGuiWindowFlags_NoMove ,
      "NoScrollbar"		  , ImGuiWindowFlags_NoScrollbar ,
      "NoScrollWithMouse"	  , ImGuiWindowFlags_NoScrollWithMouse ,
      "NoCollapse"		  , ImGuiWindowFlags_NoCollapse ,
      "AlwaysAutoResize"	  , ImGuiWindowFlags_AlwaysAutoResize ,
      "NoBackground"		  , ImGuiWindowFlags_NoBackground ,
      "NoSavedSettings"		  , ImGuiWindowFlags_NoSavedSettings ,
      "NoMouseInputs"		  , ImGuiWindowFlags_NoMouseInputs ,
      "MenuBar"			  , ImGuiWindowFlags_MenuBar ,
      "HorizontalScrollbar"	  , ImGuiWindowFlags_HorizontalScrollbar ,
      "NoFocusOnAppearing"	  , ImGuiWindowFlags_NoFocusOnAppearing ,
      "NoBringToFrontOnFocus"	  , ImGuiWindowFlags_NoBringToFrontOnFocus ,
      "AlwaysVerticalScrollbar"	  , ImGuiWindowFlags_AlwaysVerticalScrollbar ,
      "AlwaysHorizontalScrollbar" , ImGuiWindowFlags_AlwaysHorizontalScrollbar ,
      "AlwaysUseWindowPadding"	  , ImGuiWindowFlags_AlwaysUseWindowPadding ,
      "NoNavInputs"	          , ImGuiWindowFlags_NoNavInputs ,
      "NoNavFocus"		  , ImGuiWindowFlags_NoNavFocus ,
      "UnsavedDocument"		  , ImGuiWindowFlags_UnsavedDocument ,
      "NoDocking"		  , ImGuiWindowFlags_NoDocking ,
      "NoNav"			  , ImGuiWindowFlags_NoNav ,
      "NoDecoration"	          , ImGuiWindowFlags_NoDecoration ,
      "NoInputs"	  	  , ImGuiWindowFlags_NoInputs ,
      "NavFlattened"		  , ImGuiWindowFlags_NavFlattened ,
      "ChildWindow"		  , ImGuiWindowFlags_ChildWindow ,
      "Tooltip"			  , ImGuiWindowFlags_Tooltip ,
      "Popup"			  , ImGuiWindowFlags_Popup ,
      "Modal"			  , ImGuiWindowFlags_Modal ,
      "ChildMenu"		  , ImGuiWindowFlags_ChildMenu ,
      "DockNodeHost"		  , ImGuiWindowFlags_DockNodeHost ,
      // "NoClose"		  , ImGuiWindowFlags_NoClose,
      // "NoDockTab"		  , ImGuiWindowFlags_NoDockTab ,
      "DockNodeHost"		  , ImGuiWindowFlags_DockNodeHost
    );

    lua_state.new_enum(
      "ImGuiCond" ,
      "None"	     , ImGuiCond_None,
      "Always"	     , ImGuiCond_Always,
      "Once"	     , ImGuiCond_Once,
      "FirstUseEver" , ImGuiCond_FirstUseEver,
      "Appearing"    , ImGuiCond_Appearing
    );

    sol::table ImGui_bindings = lua_state.create_named_table("ImGui");
    ImGui_bindings.set_function("Begin", sol::overload(
      sol::resolve<bool(const std::string&)>(Begin) ,
      sol::resolve<std::tuple<bool , bool>(const std::string&, bool)>(Begin) ,
      sol::resolve<std::tuple<bool , bool>(const std::string&, bool, int32_t)>(Begin)
    ));
    ImGui_bindings.set_function("End" , End);

    ImGui_bindings.set_function("SetNextWindowSize", sol::overload(
      sol::resolve<void(float,float)>(SetNextWindowSize) ,
      sol::resolve<void(float,float,int)>(SetNextWindowSize)
    ));
    ImGui_bindings.set_function("SetNextWindowSizeConstraints" , SetNextWindowSizeConstraints);

    ImGui_bindings.set_function("GetContentRegionMax" , GetContentRegionMax);
    ImGui_bindings.set_function("GetContentRegionAvail" , GetContentRegionAvail);
    ImGui_bindings.set_function("GetWindowContentRegionMin" , GetWindowContentRegionMin);
    ImGui_bindings.set_function("GetWindowContentRegionMax" , GetWindowContentRegionMax);
    
    ImGui_bindings.set_function("Text" , Text);

    ImGui_bindings.set_function("Button" , Button);
    // sol::overload(
    //   sol::resolve<bool(const std::string&)>(Button) ,
    //   sol::resolve<bool(const std::string&,float,float)>(Button)
    // ));
  }

} // namespace lua_script_bindings
} // namespace other
