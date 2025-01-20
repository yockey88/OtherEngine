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

  custom_configurations = function()
    filter "configurations:Profile"
      runtime "Release"
      optimize "Full"
      symbols "On"
      
      defines { 
        "OTHER_PROFILE_BUILD",
        "TRACY_ENABLE",
        "TRACY_ON_DEMAND",
        "TRACY_CALLSTACK=10", 
      }

      files { 
        "%{wks.location}/externals/tracy/TracyClient.cpp",
      }
      includedirs { "%{wks.location}/externals/tracy" }
      
      ProcessDependencies("Release")
  end,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src",
  }
}

AddModule(oe_native)