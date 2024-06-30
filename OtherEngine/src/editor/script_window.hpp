/**
 * \file editor/script_window.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_WINDOW_HPP
#define OTHER_ENGINE_SCRIPT_WINDOW_HPP

#include <array>

#include "rendering/ui/ui_window.hpp"

namespace other {

  class ScriptWindow : public UIWindow {
    public:
      ScriptWindow(bool is_editor_script = false);

    private:
      constexpr static std::string_view cs_script_template = 
          "using System;\n"
          "using Other;\n\n"
          "class @classname : @baseobject {\n"
          "  public override void OnInitialize() {\n"
          "  }\n\n"
          "  public override void Update(float dt) {\n"
          "  }\n\n"
          "  public override void OnShutdown() {\n"
          "  }\n"
          "}";
      
      constexpr static std::string_view lua_script_template = 
          "@classname = {\n"
          "  OnInitialize = function(self) end,\n\n"
          "  OnUpdate = function(self , dt) end,\n\n"
          "  OnShutdown = function(self) end,\n\n"
          "}";

      std::array<char , 512> buffer;
      std::string name;

      bool editor_script = false;
      uint32_t ext_idx = 0;
      Opt<Path> path;
      Opt<std::string> source;

      void DrawName();
      void DrawOptions(); 

      void WriteSource();

      std::string GetSourceTemplate() const;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_WINDOW_HPP
