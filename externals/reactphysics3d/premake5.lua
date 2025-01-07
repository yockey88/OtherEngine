local reactphysics = {
  name = "reactphysics3d",
  kind = "StaticLib",
  language = "C++",
  cppdialect = "C++20",
  
  files = function()
    files {
      "./src/**.cpp" ,
      "./src/**.h"
    }
  end,
  
  include_dirs = function()
    includedirs {
      "./include"
    }
  end,


}

AddExternalProject(reactphysics)