local OtherTestEngine = {
  name = "OtherTestEngine",
  path = "./OtherTestEngine",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  
  files = function()
    files {
      "./src/**.cpp",
      "./src/**.hpp",
      "./tests/**.cpp" ,
      "./tests/**.hpp" ,
    }
  end,
  
  include_dirs = function()
    includedirs {
      "./src",
      "./tests" ,
    }
    externalincludedirs{
      "%{wks.location}/DotOther/NetCore",
      "%{wks.location}/externals/gtest/googlemock/include",
    }
  end,
  
  defines = function()
    defines {
      "OE_MODULE" ,
      "OE_TESTING_ENVIRONMENT"
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
    buildoptions { "/EHsc" , "/Zc:preprocessor" , "/Zc:__cplusplus" }
  end,
  
  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src",
    ["gtest"] = "%{wks.location}/externals/gtest/googletest/include",
  },

}



AddProject(OtherTestEngine)
