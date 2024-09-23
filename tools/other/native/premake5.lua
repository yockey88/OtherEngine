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
}

AddModule(oe_native)