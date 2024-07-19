local imgui_node_editor = {}

imgui_node_editor.name = "imgui_node_editor"
imgui_node_editor.kind = "StaticLib"
imgui_node_editor.language = "C++"
imgui_node_editor.cppdialect = "C++Latest"

imgui_node_editor.files = function()
  files{
    "./imgui_node_editor/*.cpp" ,
    "./imgui_node_editor/*.h" ,
  }
end

imgui_node_editor.include_dirs = function()
  includedirs {
    "." ,
    "../imgui/" ,
  }
end

imgui_node_editor.external_include_dirs = function()end

AddExternalProject(imgui_node_editor)
