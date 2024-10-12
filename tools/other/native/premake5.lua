local oe_native = {
  name = "engine",
  kind = "SharedLib",
  language = "C++",
  cppdialect = "C++latest",
  tdir = "./bin/%{cfg.buildcfg}",
  odir = "./bin_obj/%{cfg.buildcfg}",
  extension = ".pyd",

  files = function()
    files {
      "./src/**.cpp",
    }
  end,

  include_dirs = function()
    includedirs {
      "./src",
      "%{wks.location}/OtherEngine/src"
    }
    externalincludedirs {
      "%{wks.location}/DotOther/NetCore",
      "%{wks.location}/externals/gtest/googlemock/include"
    }
  end,

  links = function()
    libdirs { 
      PythonPaths.lib_path
    }

    links { 
      PythonPaths.lib,
      "OtherEngine"
    }
  end,

  defines = function()
    defines {
      "OE_MODULE",
    }
  end,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src",
    ["OtherEngine.Native"] = "%{wks.location}/OtherEngine/src",
  }
}

AddModule(oe_native)