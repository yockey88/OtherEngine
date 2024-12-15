local jolt = {
  name = "jolt",
  kind = "StaticLib",
  language = "C++",
  cppdialect = "C++20",
  
  files = function()
    files {
      "./Jolt/**.cpp" ,
      "./Jolt/**.h"
    }
  end,
  
  include_dirs = function()
    includedirs {
      "."
    }
  end,
}


AddExternalProject(jolt)
