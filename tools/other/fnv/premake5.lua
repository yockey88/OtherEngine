local fnv = {
  name = "fnv",
  path = "./fnv",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",

  files = function()
    files {
      "./fnv.cpp",
    }
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
  },
}

AddConsoleProject(fnv)

