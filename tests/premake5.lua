local sandbox = {}

sandbox.name = "sandbox"
sandbox.path = "./sandbox"
sandbox.kind = "ConsoleApp"
sandbox.language = "C++"
sandbox.cppdialect = "C++latest"

sandbox.files = function()
  files {
    "./sandbox/**.cpp",
    "./sandbox/**.hpp",
    "./sandbox_ui.cpp" ,
    "./sandbox_ui.hpp" ,
    "./mock_app.cpp"
  }
end

sandbox.include_dirs = function()
  includedirs {
    "./sandbox",
    "." ,
  }
end

sandbox.defines = function()
  defines {
    "OE_MODULE" ,
  }
end

sandbox.components = {}
sandbox.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
sandbox.components[""] = "%{wks.location}/externals/gtest/googlemock/include"

AddProject(sandbox)

local gl_sandbox = {}

gl_sandbox.name = "gl_sandbox"
gl_sandbox.path = "./gl_sandbox"
gl_sandbox.kind = "ConsoleApp"
gl_sandbox.language = "C++"
gl_sandbox.cppdialect = "C++latest"

gl_sandbox.files = function()
  files {
    "./gl_sandbox/**.cpp",
    "./gl_sandbox/**.hpp",
    "./sandbox_ui.cpp" ,
    "./sandbox_ui.hpp" ,
    "./mock_app.cpp"
  }
end

gl_sandbox.include_dirs = function()
  includedirs {
    "./gl_sandbox",
    "." ,
  }
end

gl_sandbox.defines = function()
  defines {
    "OE_MODULE" ,
  }
end

gl_sandbox.components = {}
gl_sandbox.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
gl_sandbox.components[""] = "%{wks.location}/externals/gtest/googlemock/include"

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

local unit_tests = {}
unit_tests.name = "unit_tests"
unit_tests.path = "./unit_tests"
unit_tests.kind = "ConsoleApp"
unit_tests.language = "C++"
unit_tests.cppdialect = "C++latest"

unit_tests.files = function()
  files {
    "./unit_tests/**.cpp" ,
  }
end

unit_tests.include_dirs = function()
  includedirs {
    "." ,
  }
end

unit_tests.components = {}
unit_tests.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
unit_tests.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"
unit_tests.components[""] = "%{wks.location}/externals/gtest/googlemock/include"

AddProject(unit_tests)