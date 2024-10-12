local yockcraft = {
  name = "yockcraft",
  path = "./yockcraft",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  disable_nuget_resolve = true,

  files = function()
    files {
      "./yockcraft/src/**.cpp" ,
      "./yockcraft/src/**.hpp" ,
    }
  end,

  include_dirs = function()
    includedirs {
      "./yockcraft/src",
    }
    externalincludedirs {
      "%{wks.location}/DotOther/NetCore",
    }
  end,

  links = function()
    libdirs {
      "./engine/%{cfg.buildcfg}" ,
    }
  end,

  defines = function()
    defines {
      "OE_MODULE"
    }
  end,

  window_configuration = function()
    includedirs {
      "./platform",
    }
    systemversion "latest"
    buildoptions { 
      "/EHsc" , 
      "/Zc:preprocessor" , 
      "/Zc:__cplusplus"
    }
    defines {
      "DOTOTHER_WINDOWS" ,
    }
  end,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src"
  }
}
AddProject(yockcraft)

include "yockcraft_modules/premake5.lua"