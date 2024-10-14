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
    systemversion "latest"
    buildoptions { 
      "/EHsc" , 
      "/Zc:preprocessor" , 
      "/Zc:__cplusplus" ,
      "/Zm10",
    }
  end,

  debug_configuration = function()
    externalincludedirs{
      "%{wks.location}/externals/gtest/googletest/include",
    }
    defines {
      "OE_TESTING_ENVIRONMENT",
    }
  end ,

  components = {
    ["DotOther.Native"] = "%{wks.location}/DotOther",
    [""] = "%{wks.location}/DotOther/NetCore",
  }
}

AddProject(OtherEngine)