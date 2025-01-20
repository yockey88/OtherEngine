local OtherTestEngine = {
  name = "OtherTestEngine",
  path = "./OtherTestEngine",
  kind = "StaticLib",
  language = "C++",
  cppdialect = "C++latest",

  files = function()
    files {
      "./src/**.cpp",
      "../OtherEngine/src/**.cpp",
      "../OtherEngine/src/**.cpp"
    }
  end,

  include_dirs = function()
    includedirs {
      "./src",
      "../OtherEngine/src",
    }
    externalincludedirs{
      "%{wks.location}/externals/gtest/googlemock/include",
    }
  end,

  defines = function()
    defines {
      "OE_MODULE" ,
      "OE_TESTING_ENVIRONMENT",
      "OTHER_DEBUG_BUILD",
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
    systemversion "latest"
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
    ["DotOther.Native"] = "%{wks.location}/DotOther",
    ["gtest"] = "%{wks.location}/externals/gtest/googletest/include",
    [""] = "%{wks.location}/DotOther/NetCore",
  },

}

AddProject(OtherTestEngine)
