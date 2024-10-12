include "./sandbox_scripts/premake5.lua"
include "./scripts/premake5.lua"

local sandbox = {
  name = "sandbox",
  path = "./sandbox",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",

  files = function()
    files {
      "./sandbox/**.cpp",
      "./sandbox/**.hpp",
      "./sandbox_ui.cpp" ,
      "./sandbox_ui.hpp" ,
      "./mock_app.cpp"
    }
  end,

  include_dirs = function()
    includedirs {
      "./sandbox",
      "." ,
    }
    externalincludedirs {
      "%{wks.location}/DotOther/NetCore",
      "%{wks.location}/externals/gtest/googlemock/include"
    }
  end,

  defines = function()
    defines {
      "OE_MODULE" ,
    }
  end,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src"
  }
}

AddProject(sandbox)

local gl_sandbox = {
  name = "gl_sandbox",
  path = "./gl_sandbox",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  
  files = function()
    files {
      "./gl_sandbox/**.cpp",
      "./gl_sandbox/**.hpp",
      "./sandbox_ui.cpp" ,
      "./sandbox_ui.hpp" ,
      "./mock_app.cpp"
    }
  end,
  
  include_dirs = function()
    includedirs {
      "./gl_sandbox",
      "." ,
    }
    externalincludedirs {
      "%{wks.location}/DotOther/NetCore", 
      "%{wks.location}/externals/gtest/googlemock/include"
    }
  end,
  
  defines = function()
    defines {
      "OE_MODULE" ,
    }
  end,
  
  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src"
  }
}

AddProject(gl_sandbox)

local unit_tests = {
  name = "unit_tests",
  path = "./unit_tests",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  
  files = function()
    files {
      "./unit_tests/**.cpp" ,
    }
  end,
  
  include_dirs = function()
    includedirs {
      "." ,
    }
    externalincludedirs {
      "%{wks.location}/DotOther/NetCore", 
      "%{wks.location}/externals/gtest/googlemock/include"
    }
  end,

  defines = function()
    defines {
      "OE_MODULE" ,
      "OE_TESTING_ENVIRONMENT"
    }
  end,
  
  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src",
    ["gtest"] = "%{wks.location}/externals/gtest/googletest/include",
  }
}

AddProject(unit_tests)