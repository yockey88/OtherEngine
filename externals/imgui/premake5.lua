local imgui = {}

imgui.name = "imgui"
imgui.kind = "StaticLib"
imgui.language = "C++"
imgui.cppdialect = "C++20"

imgui.files = function() 
  files {
    "./imgui/*.cpp" ,
    "./imgui/*.h" ,
    "imgui/backends/imgui_impl_sdl2.cpp" ,
    "imgui/backends/imgui_impl_sdl2.h" ,
    "imgui/backends/imgui_impl_opengl3.cpp" ,
    "imgui/backends/imgui_impl_opengl3.h" ,
    "imgui/misc/cpp/imgui_stdlib.cpp" ,
    "imgui/misc/cpp/imgui_stdlib.h" ,
    "imgui/misc/freetype/imgui_freetype.cpp" ,
    "imgui/misc/freetype/imgui_freetype.h"
  }
end

imgui.include_dirs = function() 
  includedirs {
    "." ,
    "./imgui" ,
    "./imgui/backends" ,
    "./imgui/misc/cpp" ,
  }
end

imgui.externalincludedirs = function() 
  externalincludedirs {
    "../sdl2/SDL2" ,
  }
end

imgui.defines = function() 
  defines { "IMGUI_DEFINE_MATH_OPERATORS" }
end

AddExternalProject(imgui)
