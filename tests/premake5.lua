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

local node_editor_tests = {}
node_editor_tests.name = "node_editor_tests"
node_editor_tests.path = "./node_editor_tests"
node_editor_tests.kind = "ConsoleApp"
node_editor_tests.language = "C++"
node_editor_tests.cppdialect = "C++latest"

node_editor_tests.files = function()
  files {
    "./node_editor_tests/main.cpp"
  }
end

node_editor_tests.include_dirs = function()
  includedirs {
    "./node_editor_tests",
  }
end

node_editor_tests.post_build_commands = function()
  filter { "system:windows" , "configurations:Debug" }
    postbuildcommands {
      '{COPY} "%{wks.location}/externals/sdl2/lib/Debug/SDL2d.dll" "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/mono-2.0-sgen.dll "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/mono-2.0-sgen.pdb "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/MonoPosixHelper.dll "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/MonoPosixHelper.pdb "%{cfg.targetdir}"',
    }

  filter { "system:windows" , "configurations:Release" }
    postbuildcommands {
      '{COPY} "%{wks.location}/externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/mono-2.0-sgen.dll "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/MonoPosixHelper.dll "%{cfg.targetdir}"',
    }
end

node_editor_tests.components = {}
node_editor_tests.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
node_editor_tests.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"

AddProject(node_editor_tests)

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