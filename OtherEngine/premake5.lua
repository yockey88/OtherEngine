local OtherEngine = {
  name = "OtherEngine",
  path = "./OtherEngine",
  kind = "StaticLib",
  language = "C++",
  cppdialect = "C++latest",

  files = function()
    files {
      "./src/**.cpp",
      "./src/**.hpp",
    }
  end,

  include_dirs = function()
    includedirs {
      "./src",
    }
  end,

  windows_configuration = function()
    files {
      "./platform/windows/**.hpp",
      "./platform/windows/**.cpp",
    }
    includedirs {
      "./platform",
    }
    buildoptions {
      "/Zm10",
    }
  end,

  debug_configuration = function()
    defines {
      "OTHER_DEBUG_BUILD"
    }
  end ,

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
    ["DotOther.Native"] = "%{wks.location}/DotOther",
    [""] = "%{wks.location}/DotOther/NetCore",
  }
}

AddProject(OtherEngine)
