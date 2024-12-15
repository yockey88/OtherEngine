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
    includedirs { "./src" }
    externalincludedirs { "%{wks.location}/DotOther/NetCore" }
  end,

  defines = function()
    defines { "OE_MODULE" }
  end,

  links = function()
    libdirs { PythonPaths.lib_path }
    links { PythonPaths.lib }
  end,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src",
  }
}

AddModule(oe_native)