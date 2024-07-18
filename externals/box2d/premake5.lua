local box2d = {}

box2d.name = "box2d"
box2d.kind = "StaticLib"
box2d.language = "C++"

box2d.files = function()
  files {
    "./src/**.cpp" ,
    "./src/**.h"
  }
end

box2d.include_dirs = function()
  includedirs {
    "." , "./box2d"
  }
end

AddExternalProject(box2d)
