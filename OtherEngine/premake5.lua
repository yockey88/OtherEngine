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
  
  windows_debug_configuration = function()
    links {
      "DbgHelp",
    }
    defines {
      "TRACY_ENABLE" ,
      "TRACY_ON_DEMAND" ,
    }
  end,
}

AddProject(OtherEngine)