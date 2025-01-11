include "./sandbox_scripts/premake5.lua"
include "./scripts/premake5.lua"
include "./misc/premake5.lua"

local sandbox = {
  name = "sandbox",
  path = "./sandbox",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  architecture = "x86_64",

  files = function()
    files {
      "./sandbox/**.cpp",
      "./sandbox/**.hpp",
    }
  end,

  include_dirs = function()
    includedirs { "./sandbox" }
    externalincludedirs { "%{wks.location}/DotOther/NetCore" }
  end,

  defines = function()
    defines { "OE_MODULE" }
  end,

  windows_filters = function()
    filter { "configurations:Release" }
      postbuildcommands {
        '{COPY} "%{wks.location}externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
        '{COPY} "%{wks.location}externals/assimp/lib/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"',
      }
    filter { "configurations:Debug" }
      postbuildcommands {
        '{COPY} "%{wks.location}externals/sdl2/lib/Debug/SDL2d.dll" "%{cfg.targetdir}"',
        '{COPY} "%{wks.location}externals/assimp/lib/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"',
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
      "./mock_app.cpp"
    }
  end,

  include_dirs = function()
    includedirs {
      "./gl_sandbox",
      ".",
    }
    externalincludedirs {
      "%{wks.location}/DotOther/NetCore",
      "%{wks.location}/externals/gtest/googlemock/include"
    }
  end,

  defines = function()
    defines { "OE_MODULE" }
  end,

  windows_filters = function()
    filter { "configurations:Release" }
      postbuildcommands {
        '{COPY} "%{wks.location}externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
        '{COPY} "%{wks.location}externals/assimp/lib/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"',
      }
    filter { "configurations:Debug" }
      postbuildcommands {
        '{COPY} "%{wks.location}externals/sdl2/lib/Debug/SDL2d.dll" "%{cfg.targetdir}"',
        '{COPY} "%{wks.location}externals/assimp/lib/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"',
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
  cross_platform_main = true,

  files = function()
    files {
      "./unit_tests/**.cpp",
      "./mock_app.cpp",
    }
  end,

  include_dirs = function()
    includedirs {
      ".",
    }
    externalincludedirs {
      "%{wks.location}/DotOther/NetCore",
      "%{wks.location}/externals/gtest/googlemock/include"
    }
  end,

  defines = function()
    defines {
      "OE_MODULE",
      "OE_TESTING_ENVIRONMENT"
    }
  end,

  windows_filters = function()
    filter { "configurations:Release" }
      postbuildcommands {
        '{COPY} "%{wks.location}externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
        '{COPY} "%{wks.location}externals/assimp/lib/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"',
      }
    filter { "configurations:Debug" }
      postbuildcommands {
        '{COPY} "%{wks.location}externals/sdl2/lib/Debug/SDL2d.dll" "%{cfg.targetdir}"',
        '{COPY} "%{wks.location}externals/assimp/lib/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"',
      }
  end,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src",
    ["OtherTestEngine"] = "%{wks.location}/OtherTestEngine/src",
    ["gtest"] = "%{wks.location}/externals/gtest/googletest/include",
  }
}

AddProject(unit_tests)

local default_sim_testing_table = {
  path = "./simulation_tests",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",

  include_dirs = function()
    includedirs {
      "."
    }
    externalincludedirs {
      "%{wks.location}/OtherEngine/src",
      "%{wks.location}/DotOther/NetCore",
      "%{wks.location}/externals/gtest/googlemock/include"
    }
  end,

  defines = function()
    defines {
      "OE_MODULE",
      "OE_TESTING_ENVIRONMENT"
    }
  end,

  components = {
    ["OtherTestEngine"] = "%{wks.location}/OtherTestEngine/src",
    ["gtest"] = "%{wks.location}/externals/gtest/googletest/include",
  }
}

local fuzzer_test = default_sim_testing_table
fuzzer_test.name = "fuzzer_test"
fuzzer_test.files = function()
  files {
    "./simulation_tests/fuzzing/fuzzing.cpp"
  }
end
AddProject(fuzzer_test)
