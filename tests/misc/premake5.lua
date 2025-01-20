local physics = {
  name = "physics_tests",
  path = "./physics",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  architecture = "x86_64",
  cross_platform_main = true,

  files = function()
    files { "./physics/*.cpp" }
  end,

  include_dirs = function()
    includedirs { "./physics" }
  end,

  defines = function()
    defines { "OE_MODULE" }
  end,

  custom_configurations = function()
    filter "configurations:Profile"
      runtime "Release"
      optimize "Full"
      symbols "On"
      
      defines { 
        "DOTOTHER_PROFILE_BUILD",
        "TRACY_ENABLE",
        "TRACY_ON_DEMAND",
        "TRACY_CALLSTACK=10", 
      }

      files { 
        "%{wks.location}/externals/tracy/TracyClient.cpp",
      }
      externalincludedirs { "%{wks.location}/externals/tracy" }
      
      ProcessDependencies("Release")
  end,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src",
  }
}

AddProject(physics)
